#include <tml/tml_parser.h>
#include <cfg/cfg.h>
#include <string.h>
#include <stdlib.h>
//#include <iostream> // only for testing

cfg::TmlParser::TmlParser()
		:mFilename(), mIfs(), mLine(), mErrorCode(0), mErrorMsg(),
		mLineNumber(0),
		mIndentChar(0),
		mIndentCharCount(1)
{
}

cfg::TmlParser::TmlParser(const std::string& filename)
	:TmlParser()
{
	setFilename(filename);
}

cfg::TmlParser::~TmlParser()
{
	if (mIfs.is_open()) {
		mIfs.close();
	}
}

void cfg::TmlParser::reset()
{
	if (mIfs.is_open()) {
		mIfs.close();
	}
	mIfs.clear();
	mLine.clear();
	mErrorCode = 0;
	mErrorMsg.clear();
	mLineNumber = 0;
	mIndentChar = 0;
	mIndentCharCount = 1;
}

bool cfg::TmlParser::setFilename(const std::string& filename)
{
	reset();

	mFilename = filename;
	return true;
}

bool cfg::TmlParser::begin()
{
	mLineNumber = 0;
	if (mIfs.is_open()) {
		mIfs.clear(); // clear error flags
		mIfs.seekg(0, mIfs.beg);
		if (mIfs.fail()) {
			mErrorMsg = "Can't seek to the beginning.";
			mErrorCode = -1;
			return false;
		}
		return true;
	}
	mIfs.open(mFilename.c_str(), std::ifstream::in);
	if (mIfs.fail()) {
		mErrorMsg = "Can't open file.";
		mErrorCode = -2;
		return false;
	}
	return true;
}

int cfg::TmlParser::getNextTmlEntry(NameValuePair& entry)
{
	if (!mIfs.is_open()) {
		mErrorCode = -3;
		return -2;
	}
	getline(mIfs, mLine);
	if (mIfs.eof()) {
		mIfs.close();
		// no error code and no return here because if last line with content
		// has no line break at the end then eof() is already true, but
		// the line should not be discard --> no error and no return.
		//mErrorCode = -3; // would be wrong here
		//return -2; // would be wrong here
	}
	if (mIfs.fail()) {
		mErrorCode = -3;
		return -2;
	}
	++mLineNumber;
	return getNextTmlEntry(mLine, entry, mLineNumber);
}

int cfg::TmlParser::getNextTmlEntry(std::string& utf8Line, NameValuePair& entry,
		int lineNumber)
{
	entry.clear();
	entry.mName.mLineNumber = lineNumber;
	entry.mValue.mLineNumber = lineNumber;

	std::size_t len = utf8Line.length();
	if (len > 0) {
		if (utf8Line[len - 1] == '\n') {
			--len;
		}
	}
	if (len > 0) {
		if (utf8Line[len - 1] == '\r') {
			--len;
		}
	}
	utf8Line.erase(len);

	if (!mIndentChar && len > 0 && (utf8Line[0] == ' ' || utf8Line[0] == '\t')) {
		mIndentChar = utf8Line[0];
		// mIndentCharCount already has the value 1 from init
		for (; mIndentCharCount < len && utf8Line[mIndentCharCount] == mIndentChar; ++mIndentCharCount)
			;
	}
	unsigned int deep = 0;
	unsigned int i = 0;
	if (mIndentChar) {
		for (; i < len && utf8Line[i] == mIndentChar; ++i)
			;
		deep = i / mIndentCharCount;
		if (i % mIndentCharCount) {
			mErrorMsg = "Wrong indention. Is not a multiple of the indention count";
			return -1;
		}
	}
	if (i >= len) {
		// a empty line with or without indention
		entry.mName.mOffset = i;
		entry.mValue.mOffset = i;
		entry.mDeep = deep;
		return deep;
	}
	char ch = utf8Line[i];
	if (ch == ' ' || ch == '\t') {
		mErrorMsg = "Space/blank or tab is not allowed at the beginning after the indention.";
		return -1;
	}
	if (ch == '=') {
		mErrorMsg = "= is not allowed at the beginning after the indention.";
		return -1;
	}
	else if (ch == '#') {
		entry.mName.setComment(utf8Line.c_str() + i + 1);
		entry.mName.mLineNumber = lineNumber;
		entry.mName.mOffset = i + 1;
		entry.mValue.mOffset = len;
		entry.mDeep = deep;
		return deep;
	}

	//unsigned int lineStartIndex = i; // start of line without indention

	unsigned int valueCount = 1;
	unsigned int wordCountPerValue = 0;

	Value* array = &entry.mName;
	Value* value = &entry.mName;
	while (i < len) {
		ch = utf8Line[i];
		if (ch == '=') {
			++i;
			// forward to next word beginning
			for (; i < len && (utf8Line[i] == ' ' || utf8Line[i] == '\t'); ++i)
				;
			if (i >= len) {
				mErrorMsg = "= is not allowed at the end of a line.";
				return -1;
			}
			ch = utf8Line[i];
			++valueCount;
			if (valueCount > 2) {
				mErrorMsg = "Only two values are allowed for a value pair (only one = is allowed).";
				return -1;
			}
			// now valueCount can only be 2
			array = &entry.mValue;
			value = &entry.mValue;
			wordCountPerValue = 0;
		}

		unsigned int wordStartIndex = i;
		unsigned int dotCount = 0;
		unsigned int digitCount = 0;
		bool isNumber = true;
		bool isOnlyText = false;
		if (ch == '"') {
			unsigned int di = i; // destination index / for copy
			++i;
			//char prevCh = ch;
			//int moveDiff = 1;
			bool isEscSeq = false;
			for (; i < len; ++i) {
				ch = utf8Line[i];
				if (ch == '"') {
					if (isEscSeq) {
						isEscSeq = false;
					}
					else {
						++i;
						isOnlyText = true;
						break;
					}
				}
				else if (ch == '\\') {
					if (!isEscSeq) {
						isEscSeq = true;
						continue;
					}
					isEscSeq = false;
				}
				else if (isEscSeq) {
					switch (ch) {
						case 't':
							ch = '\t';
							isEscSeq = false;
							break;
						case 'n':
							ch = '\n';
							isEscSeq = false;
							break;
					}
				}
				if (isEscSeq) {
					mErrorMsg = "Start escape sequence with \\ but a wrong character follows.";
					return -1;
				}
				utf8Line[di] = ch;
				//prevCh = ch;
				++di;
			}
			if (!isOnlyText) {
				mErrorMsg = "No closing \" for the end of the text.";
				return -1;
			}
			// ii < len doesn't need to be checked because i can be only len but not greater.
			for (unsigned int ii = di; ii < i; ++ii) {
				utf8Line[ii] = ' ';
			}
			i = di; // set to the first blank ;-)
		}
		else {
			if (ch == '+' || ch == '-') {
				++i;
				if (i < len) {
					ch = utf8Line[i];
				}
			}
			for (; i < len && utf8Line[i] != ' ' && utf8Line[i] != '\t' &&
				   utf8Line[i] != '='; ++i) {
				ch = utf8Line[i];
				if (ch >= '0' && ch <= '9') {
					++digitCount;
				}
				else if (ch == '.') {
					++dotCount;
				}
				else {
					isNumber = false;
				}
			}
		}
		const char* word = utf8Line.c_str() + wordStartIndex;
		if (i < len) {
			ch = utf8Line[i];
			utf8Line[i] = '\0';
		}
		++wordCountPerValue;
		if (wordCountPerValue == 2) {
			Value cv = *value;
			value->setArray();
			value->mLineNumber = cv.mLineNumber;
			value->mOffset = cv.mOffset;
			value->mArray.push_back(cv);
		}
		if (wordCountPerValue >= 2) {
			array->mArray.push_back(Value());
			array->mArray.back().mFilename = array->mFilename;
			value = &array->mArray.back();
		}

		if (isOnlyText) {
			// it can be also a text if isOnlyText is false.
			// If isOnlyText is false it only can be a text if no other
			// cases (number, boolean, etc.) accepted.
			value->setText(word);
		}
		else if (isNumber && digitCount && dotCount == 0) {
			//std::cout << "type: int" << std::endl;
			value->setInteger(atoi(word), 10);
		}
		else if (isNumber && digitCount && dotCount == 1) {
			//std::cout << "type: float" << std::endl;
			value->setFloatingPoint(static_cast<float>(atof(word)));
		}
		else if (!strcmp(word, "true")) {
			//std::cout << "type: bool" << std::endl;
			value->setBool(true);
		}
		else if (!strcmp(word, "false")) {
			//std::cout << "type: bool" << std::endl;
			value->setBool(false);
		}
		else if (!strcmp(word, "null")) {
			//std::cout << "type: null" << std::endl;
			value->setNull();
		}
		else {
			//std::cout << "type: text" << std::endl;
			value->setText(word);
		}
		value->mLineNumber = lineNumber;
		value->mOffset = wordStartIndex + 1;

		//std::cout << "word: '" << word << "', i: " << i << ", len: " << len << std::endl;
		if (i < len) {
			utf8Line[i] = ch;
		}

		// forward to next word beginning
		for (; i < len && (utf8Line[i] == ' ' || utf8Line[i] == '\t'); ++i)
			;
	}
	//std::cout << "TODO: " << deep << " " << utf8Line << std::endl;
	entry.mDeep = deep;
	if (valueCount == 1) {
		entry.mValue.mOffset = i + 1;
	}
	return deep;
}

bool cfg::TmlParser::getAsTree(NameValuePair &root,
		bool inclEmptyLines, bool inclComments)
{
	root.clear();
	root.mName.setText(mFilename);
	if (!getAsTree(root.mValue, inclEmptyLines, inclComments)) {
		root.clear();
		return false;
	}
	return true;
}

bool cfg::TmlParser::getAsTree(Value &root,
		bool inclEmptyLines, bool inclComments)
{
	root.clear();
	if (!begin()) {
		// set no error message because this is already done by begin()
		root.clear();
		return false;
	}
	std::shared_ptr<const std::string> filenamePtr = std::make_shared<const std::string>(mFilename);

	NameValuePair cfgPair;
	cfgPair.mName.mFilename = filenamePtr;
	cfgPair.mValue.mFilename = filenamePtr;
	std::vector<Value*> stack;
	stack.reserve(10);
	root.setObject();
	root.mLineNumber = 1;
	root.mOffset = 0;
	stack.push_back(&root);
	int deep = 0;
	int prevDeep = 0;
	unsigned int currentContiguousEmptyOrCommentCount = 0;

	while ((deep = getNextTmlEntry(cfgPair)) >= 0) {

		if (!cfgPair.isEmptyOrComment()) {
			if (deep > prevDeep) {
#if 1
				// should not be possible
				if (cfgPair.isEmpty()) {
					mErrorMsg = "Increase the deep with an empty line is not allowed.";
					root.clear();
					return false;
				}
#endif
				if (deep > prevDeep + 1) {
					mErrorMsg = "Can't increase the deep more than one per entry.";
					root.clear();
					return false;
				}
				if (stack.back()->mObject.empty()) {
					mErrorMsg = "No parent entry exist (should not be possible).";
					root.clear();
					return false;
				}

				std::vector<NameValuePair> tmp;

				int lineNumber = mLineNumber;

				if (currentContiguousEmptyOrCommentCount > 0) {
					std::vector<NameValuePair>& obj = stack.back()->mObject;
					if (obj.size() <= currentContiguousEmptyOrCommentCount) {
						mErrorMsg = "No parent exist without empty lines or comments.";
						root.clear();
						return false;
					}

					tmp.reserve(currentContiguousEmptyOrCommentCount);
					unsigned int startIndex = obj.size() - currentContiguousEmptyOrCommentCount;
					for (unsigned int i = 0; i < currentContiguousEmptyOrCommentCount; ++i) {
						tmp.push_back(obj[startIndex + i]);
					}
					if (obj[startIndex].mName.mLineNumber >= 0) {
						lineNumber = obj[startIndex].mName.mLineNumber;
					}
					obj.resize(startIndex); // truncate --> drop the moved entries
				}

				// check again after move
				if (stack.back()->mObject.empty()) {
					mErrorMsg = "No parent entry exist after moved entries (should not be possible).";
					root.clear();
					return false;
				}

				if (stack.back()->mObject.back().mName.isEmpty()) {
					mErrorMsg = "The name of the parent is empty.";
					root.clear();
					return false;
				}
				if (stack.back()->mObject.back().mName.isComment()) {
					mErrorMsg = "The name of the parent is a comment which is not allowed.";
					root.clear();
					return false;
				}
				if (!stack.back()->mObject.back().mValue.isEmpty()) {
					mErrorMsg = "The value of the parent is not empty (no = is allowed at parent).";
					root.clear();
					return false;
				}


				stack.push_back(&stack.back()->mObject.back().mValue);
				stack.back()->setObject();
				stack.back()->mLineNumber = lineNumber;
				stack.back()->mOffset = deep * mIndentCharCount;
				if (!tmp.empty()) {
					// copy is no problem because the new object is empty
					stack.back()->mObject = std::move(tmp);
				}
				prevDeep = deep;
			} else if (deep < prevDeep) {
				for (int i = prevDeep; i > deep; --i) {
					stack.pop_back();
				}
				prevDeep = deep;
			}

			currentContiguousEmptyOrCommentCount = 0;
		}

		if (!cfgPair.isEmptyOrComment() ||
				(inclEmptyLines && cfgPair.isEmpty()) ||
				(inclComments && cfgPair.isComment())) {

			if (cfgPair.isEmptyOrComment()) {
				++currentContiguousEmptyOrCommentCount;
			}

			stack.back()->mObject.push_back(cfgPair);
		}
	}
	if (deep == -1) {
		root.clear();
		//LOGE("parse error at line %u\n", mLineNumber);
		//LOGE("error: %s\n", getExtendedErrorMsg().c_str());
		return false;
	}
	// deep should be -2 for end of file and not -1 which is a error
	return true;
}

std::string cfg::TmlParser::getExtendedErrorMsg() const
{
	return mFilename + ":" + std::to_string(mLineNumber) + ": " + mErrorMsg;
}

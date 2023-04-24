#include <tml/tml_parser.h>
#include <cfg/cfg.h>
#include <string.h>
#include <stdlib.h>
//#include <iostream>

namespace cfg
{
	namespace
	{
		/**
		 * Find all continuous empties and comments at the end of an object or an array
		 * and check if they should moved to the parent.
		 * @param cfg Must be an object or an array.
		 * @param curDeep Current deep.
		 * @return Start index for moving. If nothing is to move then the size of
		 *         the object or array is returned.
		 */
		std::size_t getMoveIndexForEmptyAndComment(const Value& cfg, int curDeep)
		{
			int size = -1;
			int emptyCommentCount = 0;
			int moveIndex = -1;
			if (cfg.isObject()) {
				const std::vector<NameValuePair>& obj = cfg.mObject;
				size = obj.size();
				int i = -1;
				for (i = size - 1; i >= 0; --i) {
					if (!obj[i].isEmptyOrComment()) {
						// --> no empty and no comment
						break;
					}
				}
				emptyCommentCount = size - i - 1;
				int emptyIndex = size;
				int commentIndex = size;
				for (int i = size - emptyCommentCount; i < size; ++i) {
					if (obj[i].mDeep < curDeep) {
						if (obj[i].isEmpty()) {
							emptyIndex = i;
						}
						if (obj[i].isComment()) {
							commentIndex = i;
							break;
						}
					}
					else {
						if (obj[i].isComment()) {
							emptyIndex = size;
						}
					}
				}
				moveIndex = std::min(emptyIndex, commentIndex);
			}
			else if (cfg.isArray()) {
				const std::vector<Value>& array = cfg.mArray;
				size = array.size();
				int i = -1;
				for (i = size - 1; i >= 0; --i) {
					if (!array[i].isEmpty() && !array[i].isComment()) {
						// --> no empty and no comment
						break;
					}
				}
				emptyCommentCount = size - i - 1;
				int emptyIndex = size;
				int commentIndex = size;
				for (int i = size - emptyCommentCount; i < size; ++i) {
					if (array[i].mNvpDeep < curDeep) {
						if (array[i].isEmpty()) {
							emptyIndex = i;
						}
						if (array[i].isComment()) {
							commentIndex = i;
							break;
						}
					}
					else {
						if (array[i].isComment()) {
							emptyIndex = size;
						}
					}
				}
				moveIndex = std::min(emptyIndex, commentIndex);
			}
			else {
				//std::cout << cfg.getFilenameAndPosition() << ": no object and no array" << std::endl;
				return 0;
			}
#if 0
			std::cout << cfg.getFilenameAndPosition() <<
					", cur deep " << curDeep <<
					", size " << size <<
					", empty comment cnt " << emptyCommentCount <<
					", move count " << (size - moveIndex) <<
					", move idx " << moveIndex << std::endl;
#endif
			return moveIndex;
		}

		/**
		 * Only empty and comment are supported!
		 */
		void moveFromChildToParent(Value& child, Value& parent, std::size_t moveIndex)
		{
			if (!parent.isObject() && !parent.isArray()) {
				return;
			}

			std::vector<Value> tmp;
			if (child.isObject()) {
				std::vector<NameValuePair>& obj = child.mObject;
				std::size_t size = obj.size();
				if (moveIndex >= size) {
					return;
				}
				for (std::size_t i = moveIndex; i < size; ++i) {
					tmp.push_back(std::move(obj[i].mName));
				}
				obj.resize(moveIndex);
			}
			else if (child.isArray()) {
				std::vector<Value>& array = child.mArray;
				std::size_t size = array.size();
				if (moveIndex >= size) {
					return;
				}
				for (std::size_t i = moveIndex; i < size; ++i) {
					tmp.push_back(std::move(array[i]));
				}
				array.resize(moveIndex);
			}

			if (tmp.empty()) {
				return;
			}

			if (parent.isObject()) {
				std::vector<NameValuePair>& obj = parent.mObject;
				for (Value& val : tmp) {
					NameValuePair nvp;
					nvp.mDeep = val.mNvpDeep;
					nvp.mName = std::move(val);
					obj.push_back(nvp);
				}
			}
			else if (parent.isArray()) {
				std::vector<Value>& array = parent.mArray;
				for (Value& val : tmp) {
					array.push_back(std::move(val));
				}
			}
		}
	}
}
cfg::TmlParser::TmlParser()
		:mReadFromFile(false), mFilename(), mStrBuffer(), mIfs(), mIss(),
		mInStream(&mIss), mLine(), mErrorCode(0), mErrorMsg(),
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
	mIss.str(std::string());
	mIfs.clear();
	mIss.clear();
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

	mReadFromFile = true;
	mFilename = filename;
	mStrBuffer.clear();
	mInStream = &mIfs;
	return true;
}

bool cfg::TmlParser::setStringBuffer(const std::string& pseudoFilename,
		const std::string& strBuffer)
{
	reset();

	mReadFromFile = false;
	mFilename = pseudoFilename;
	mStrBuffer = strBuffer;
	mInStream = &mIss;
	return true;
}

bool cfg::TmlParser::begin()
{
	mLineNumber = 0;
	if (!mReadFromFile) {
		mIss.str(mStrBuffer);
		return true;
	}

	// --> read from file
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
	return getNextTmlEntry(entry, nullptr, nullptr);
}

int cfg::TmlParser::getNextTmlEntry(NameValuePair& entry, std::string* outLine, int* outLineNumber)
{
	if (mReadFromFile) {
		// --> file is used
		if (!mIfs.is_open()) {
			mErrorCode = -3;
			return -2;
		}
	}
	getline(*mInStream, mLine);
	if (mReadFromFile && mIfs.eof()) {
		mIfs.close();
		// no error code and no return here because if last line with content
		// has no line break at the end then eof() is already true, but
		// the line should not be discard --> no error and no return.
		//mErrorCode = -3; // would be wrong here
		//return -2; // would be wrong here
	}
	if (mInStream->fail()) {
		mErrorCode = -3;
		return -2;
	}
	++mLineNumber;
	if (outLine) {
		*outLine = mLine;
	}
	if (outLineNumber) {
		*outLineNumber = mLineNumber;
	}
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
		entry.mName.mNvpDeep = deep;
		entry.mValue.mOffset = i;
		entry.mValue.mNvpDeep = deep;
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
		entry.mName.mNvpDeep = deep;
		entry.mValue.mOffset = static_cast<int>(len);
		entry.mValue.mNvpDeep = deep;
		entry.mDeep = deep;
		return deep;
	}

	//unsigned int lineStartIndex = i; // start of line without indention

	unsigned int valueCount = 1;
	unsigned int wordCountPerValue = 0;
	bool isEmptyArray = false;
	bool isEmptyObject = false;

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
			isEmptyArray = false;
			isEmptyObject = false;
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
			if (isEmptyArray) {
				mErrorMsg = "[] is not allowed in a single line array.";
				return -1;
			}
			if (isEmptyObject) {
				mErrorMsg = "{} is not allowed in a single line array.";
				return -1;
			}
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
			// In this case isOnlyText is true which means it was a text
			// with quotes like "foo"
			value->setText(word, true);
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
		else if (!strcmp(word, "[]")) {
			if (wordCountPerValue > 1) {
				mErrorMsg = "[] is not allowed in a single line array.";
				return -1;
			}
			value->setArray();
			isEmptyArray = true;
		}
		else if (!strcmp(word, "{}")) {
			if (wordCountPerValue > 1) {
				mErrorMsg = "{} is not allowed in a single line array.";
				return -1;
			}
			value->setObject();
			isEmptyObject = true;
		}
		else {
			// it's a text without quotes like foo instead of "foo"
			//std::cout << "type: text" << std::endl;
			value->setText(word, false);
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
	//std::cout << "deep: " << deep << ", line: " << utf8Line << std::endl;
	entry.mName.mNvpDeep = deep;
	entry.mValue.mNvpDeep = deep; // also set if valueCount is 0 --> no value/empty value also stores the deep!
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
	/**
	 * Counts the empty lines and comments at the beginning of a new object/section
	 * or at the beginning of a multiple line array.
	 * Why is this important?
	 * TML Example:
	 * 1: entry1
	 * 2: entry2
	 * 3:     # this is a comment and a empty line is followed
	 * 4:
	 * 5:     sub-entry1
	 * 6: entry3
	 * The comment and empty line must be children of entry2!
	 * But this is not really simple because the count of indention steps of
	 * the comment and the empty line can be any value (is allowed).
	 * Only sub-entry1 defines thats the comment and empty line are
	 * children of entry2. If sub-entry1 doesn't exist then the comment
	 * and empty line must be included into the root object between entry2
	 * and entry3.
	 * For this there is a extra logic which moves the comments and empty lines
	 * to the correct object (or array). For this std::vector<Value> tmp; is used.
	 */
	std::size_t currentContiguousEmptyOrCommentCount = 0;

	/**
	 * A parsed line as name-value-pair can be added
	 * into an object or
	 * into a multiple line array (In this case the value of the name-value-pair must be empty).
	 */
	while ((deep = getNextTmlEntry(cfgPair)) >= 0) {

		if (!cfgPair.isEmptyOrComment()) {
			if (deep > prevDeep) {
#if 1
				// should not be possible because of above: if (!cfgPair.isEmptyOrComment()) {...
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
				if (stack.back()->mObject.empty() && stack.back()->mArray.empty()) {
					mErrorMsg = "No parent entry exist (should not be possible).";
					root.clear();
					return false;
				}

				std::vector<Value> tmp;
				int lineNumber = mLineNumber;
				bool childIsArrayEntry = false;

				if (stack.back()->isObject() && !stack.back()->mObject.empty()) {
					// --> parent is an object

					if (currentContiguousEmptyOrCommentCount > 0) {
						std::vector<NameValuePair>& obj = stack.back()->mObject;
						if (obj.size() <= currentContiguousEmptyOrCommentCount) {
							mErrorMsg = "No parent exist without empty lines or comments.";
							root.clear();
							return false;
						}

						tmp.reserve(currentContiguousEmptyOrCommentCount);
						std::size_t startIndex = obj.size() -
								currentContiguousEmptyOrCommentCount;
						for (std::size_t i = 0;
								i < currentContiguousEmptyOrCommentCount; ++i) {
							tmp.push_back(obj[startIndex + i].mName);
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
					if (stack.back()->mObject.back().mName.isArray() &&
							stack.back()->mObject.back().mName.mArray.empty()) {
						mErrorMsg = "The name of the parent is an empty array which is not allowed.";
						root.clear();
						return false;
					}
					if (stack.back()->mObject.back().mName.isObject() &&
							stack.back()->mObject.back().mName.mObject.empty()) {
						mErrorMsg = "The name of the parent is an empty object which is not allowed.";
						root.clear();
						return false;
					}

					if (stack.back()->mObject.back().mValue.isArray()) {
						if (!stack.back()->mObject.back().mValue.mArray.empty()) {
							mErrorMsg = "The value of the parent is a non empty array. Only = [] is allowed for an array with multiple lines.";
							root.clear();
							return false;
						}
						// --> an empty array --> array with multiple lines
						childIsArrayEntry = true;
						stack.back()->mObject.back().mValue.clear();
					}
					else if (!stack.back()->mObject.back().mValue.isEmpty()) {
						// --> not an empty array but also not empty --> not allowed
						mErrorMsg = "The value of the parent is not empty (no = is allowed at parent, excepted = []).";
						root.clear();
						return false;
					}

					stack.push_back(&stack.back()->mObject.back().mValue);
				}
				else if (stack.back()->isArray() && !stack.back()->mArray.empty()) {
					// --> parent is an array

					if (currentContiguousEmptyOrCommentCount > 0) {
						std::vector<Value>& array = stack.back()->mArray;
						if (array.size() <= currentContiguousEmptyOrCommentCount) {
							mErrorMsg = "No parent exist without empty lines or comments.";
							root.clear();
							return false;
						}

						tmp.reserve(currentContiguousEmptyOrCommentCount);
						std::size_t startIndex = array.size() -
								currentContiguousEmptyOrCommentCount;
						for (unsigned int i = 0;
								i < currentContiguousEmptyOrCommentCount; ++i) {
							tmp.push_back(array[startIndex + i]);
						}
						if (array[startIndex].mLineNumber >= 0) {
							lineNumber = array[startIndex].mLineNumber;
						}
						array.resize(startIndex); // truncate --> drop the moved entries
					}

					// check again after move
					if (stack.back()->mArray.empty()) {
						mErrorMsg = "No parent entry exist after moved entries (should not be possible).";
						root.clear();
						return false;
					}

					if (stack.back()->mArray.back().isEmpty()) {
						mErrorMsg = "Parent is empty.";
						root.clear();
						return false;
					}
					if (stack.back()->mArray.back().isComment()) {
						mErrorMsg = "The parent is a comment which is not allowed.";
						root.clear();
						return false;
					}

					// only an empty object or an empty array as child is here allowed!
					if (stack.back()->mArray.back().isArray()) {
						if (!stack.back()->mArray.back().mArray.empty()) {
							mErrorMsg = "The parent must be an empty array with [].";
							root.clear();
							return false;
						}
						childIsArrayEntry = true;
					}
					else if (stack.back()->mArray.back().isObject()) {
						if (!stack.back()->mArray.back().mObject.empty()) {
							mErrorMsg = "The parent must be an empty object with {}.";
							root.clear();
							return false;
						}
					}
					else {
						mErrorMsg = "The parent must be use [] or {} to add a child to an array.";
						root.clear();
						return false;
					}
					// --> now can only be an empty object or an empty array

					stack.push_back(&stack.back()->mArray.back());
				}
				else {
					mErrorMsg = "No parent entry exist (should not be possible, again).";
					root.clear();
					return false;
				}

				if (childIsArrayEntry) {
					stack.back()->setArray();
				}
				else {
					stack.back()->setObject();
				}
				stack.back()->mLineNumber = lineNumber;
				stack.back()->mOffset = deep * mIndentCharCount;
				if (!tmp.empty()) {
					// copy is no problem because the new object is empty
					//std::cout << "move tmp with " << tmp.size() << " entries." << std::endl; // only for info
					if (childIsArrayEntry) {
						stack.back()->mArray = std::move(tmp);
					}
					else {
						std::size_t tmpSize = tmp.size();
						for (std::size_t i = 0; i < tmpSize; ++i) {
							NameValuePair nvp;
							nvp.mName = std::move(tmp[i]);
							nvp.mDeep = nvp.mName.mNvpDeep;
							nvp.mValue.mNvpDeep = nvp.mName.mNvpDeep;
							stack.back()->mObject.emplace_back(nvp);
						}
					}
				}
				prevDeep = deep;
			}
			else if (deep < prevDeep) {
				// check to move empties or comments to parent if deep is lower than child
				// also shrink stack
				for (int curDeep = prevDeep; curDeep > deep; --curDeep) {
					std::size_t moveIndex = getMoveIndexForEmptyAndComment(*stack.back(), curDeep);
					Value* child = stack.back();
					stack.pop_back();
					moveFromChildToParent(*child, *stack.back(), moveIndex);
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

			if (stack.back()->isObject()) {
				if (cfgPair.mName.isArray() && cfgPair.mName.mArray.empty()) {
					mErrorMsg = "An empty array as name of a name-value-pair is not allowed.";
					root.clear();
					return false;
				}
				if (cfgPair.mName.isObject() && cfgPair.mName.mObject.empty()) {
					mErrorMsg = "An empty object as name of a name-value-pair is not allowed.";
					root.clear();
					return false;
				}

				stack.back()->mObject.push_back(cfgPair);
			}
			else if (stack.back()->isArray()) {
				if (!cfgPair.mValue.isEmpty()) {
					mErrorMsg = "An array can only store a value as element and no name value pair.";
					root.clear();
					return false;
				}
				stack.back()->mArray.push_back(cfgPair.mName);
			}
			else {
				mErrorMsg = "The parent must be an object or an array.";
				root.clear();
				return false;
			}
		}
	}
	if (deep == -1) {
		root.clear();
		//LOGE("parse error at line %u\n", mLineNumber);
		//LOGE("error: %s\n", getExtendedErrorMsg().c_str());
		return false;
	}
	//std::cout << "prev deep: before " << prevDeep << std::endl;
	// check to move empties or comments to parent if deep is lower than child
	// also shrink stack
	while (prevDeep > 0) {
		std::size_t moveIndex = getMoveIndexForEmptyAndComment(*stack.back(), prevDeep);
		Value* child = stack.back();
		stack.pop_back();
		moveFromChildToParent(*child, *stack.back(), moveIndex);
		--prevDeep;
	}
	//std::cout << "prev deep: after " << prevDeep << std::endl;
	// deep should be -2 for end of file and not -1 which is a error
	return true;
}

std::string cfg::TmlParser::getExtendedErrorMsg() const
{
	return mFilename + ":" + std::to_string(mLineNumber) + ": " + mErrorMsg;
}

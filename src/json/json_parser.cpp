#include <json/json_parser.h>
#include <cfg/cfg.h>
#include <vector>
#include <fstream>
#include <map>

#define JSMN_STRICT
#include <json/jsmn.h>

namespace cfg
{
	namespace
	{
		/**
		 * Dump the token t and its children into value val.
		 * @return Count of dumped values/tokens.
		 */
		int dumpToValue(Value& val,
				const std::shared_ptr<const std::string>& filenamePtr,
				const char* js, jsmntok_t* t, size_t count, int indent)
		{
			int i, j;
			jsmntok_t* key;
			if (count == 0) {
				return 0;
			}
			val.clear();
			val.mFilename = filenamePtr;
			switch (t->type) {
				case JSMN_UNDEFINED:
					return 0;
				case JSMN_OBJECT:
					val.setObject();
					val.mObject.resize(t->size);
					j = 0;
					for (i = 0; i < t->size; i++) {
						val.mObject[i].mDeep = indent;
						key = t + 1 + j;
						j += dumpToValue(val.mObject[i].mName, filenamePtr,
								js, key, count - j, indent + 1);
						if (key->size > 0) {
							j += dumpToValue(val.mObject[i].mValue, filenamePtr,
									js, t + 1 + j, count - j, indent + 1);
						}
					}
					return j + 1;
				case JSMN_ARRAY:
					val.setArray();
					val.mArray.resize(t->size);
					j = 0;
					for (i = 0; i < t->size; i++) {
						j += dumpToValue(val.mArray[i], filenamePtr,
								js, t + 1 + j, count - j, indent + 1);
					}
					return j + 1;
				case JSMN_STRING: {
					//printf("'%.*s'", t->end - t->start, js + t->start);
					std::string text;
					text.insert(text.end(), js + t->start, js + t->end);
					val.setText(text);
					return 1;
				}
				case JSMN_PRIMITIVE: {
					//printf("%.*s", t->end - t->start, js + t->start);
					std::string text;
					text.insert(text.end(), js + t->start, js + t->end);
					if (text == "null") {
						val.setNull();
					} else if (text == "true" || text == "false") {
						val.setBool(text == "true");
					} else {
						std::size_t len = text.length();
						std::size_t i = 0;
						unsigned int signCount = 0;
						unsigned int digitCount = 0;
						if (len > 0 && (text[0] == '+' || text[0] == '-')) {
							++i;
							signCount = 1;
						}
						for (; i < len; ++i) {
							if (text[i] >= '0' && text[i] <= '9') {
								++digitCount;
							}
						}
						if (signCount + digitCount == len) {
							// --> integer value
							val.setInteger(atoi(text.c_str()), 10);
						}
						else {
							// --> floating point
							val.setFloatingPoint(static_cast<float>(atof(text.c_str())));
						}
					}
					return 1;
				}
			}
			return 0;
		}
	}
}

cfg::JsonParser::JsonParser()
	:mFilename(), mErrorMsg(), mLineNumber(0)
{
}

cfg::JsonParser::JsonParser(const std::string& filename)
	:JsonParser()
{
	setFilename(filename);
}

cfg::JsonParser::~JsonParser()
{
}

void cfg::JsonParser::reset()
{
	mErrorMsg.clear();
	mLineNumber = 0;
}

bool cfg::JsonParser::setFilename(const std::string& filename)
{
	reset();

	mFilename = filename;
	return true;
}

bool cfg::JsonParser::getAsTree(NameValuePair &root)
{
	root.clear();
	root.mName.setText(mFilename);
	if (!getAsTree(root.mValue)) {
		root.clear();
		return false;
	}
	return true;
}

bool cfg::JsonParser::getAsTree(Value &root)
{
	return getAsTree(root, mFilename, mLineNumber, mErrorMsg);
}

bool cfg::JsonParser::getAsTree(Value &root, const std::string& filename,
		unsigned int& outLineNumber, std::string& outErrorMsg)
{
	root.clear();

	std::ifstream ifs;
	ifs.open(filename, std::ifstream::in);
	if (ifs.fail()) {
		outLineNumber = 0;
		outErrorMsg = "Can't open file.";
		return false;
	}

	std::map<std::size_t /* total byte offset of line start */, unsigned int /* line number */> lines;
	std::string line;
	std::vector<char> fullContent;
	unsigned int lineCount = 0;
	for (;;) {
		getline(ifs, line);
		if (ifs.fail() && !ifs.eof()) {
			outLineNumber = lineCount;
			outErrorMsg = "Can't read the full content of the file.";
			return false;
		}
		if (ifs.eof() && line.empty()) {
			break;
		}
		++lineCount;
		lines[fullContent.size()] = lineCount;
		fullContent.insert(fullContent.end(), line.begin(), line.end());
		fullContent.push_back('\n');
		//std::cout << "Add line " << lineNumber << " length " << line.size() << std::endl;
		if (ifs.eof()) {
			break;
		}
	}
	ifs.close();
	fullContent.push_back('\0'); // termination --> char-array is a compatible c-string

	jsmn_parser p;
	jsmn_init(&p); // Prepare parser

	std::vector<jsmntok_t> tok;
	tok.resize(128);

	int rv = 0;
	do {
		rv = jsmn_parse(&p, fullContent.data(), fullContent.size() - 1, tok.data(), tok.size());
		if (rv < 0) {
			switch (rv) {
				case JSMN_ERROR_INVAL:
				case JSMN_ERROR_PART:
				{
					const auto it = lines.upper_bound(p.pos);
					outLineNumber = (it == lines.end()) ?
							lineCount : (it->second - 1);
					outErrorMsg =
							(rv == JSMN_ERROR_INVAL) ?
							"Bad token, JSON string is corrupted." :
							"JSON string is too short, expecting more JSON data";
					return false;
				}
				case JSMN_ERROR_NOMEM:
					tok.resize(tok.size() * 2);
					//std::cout << "not enough tokens, JSON string is too large --> resize tokens" << std::endl;
					break;
			}
		}
		//std::cout << "parser returned: " << rv << std::endl;
	} while (rv == JSMN_ERROR_NOMEM);

	if (rv < 0) {
		// should not really be possible.
		outLineNumber = lineCount;
		outErrorMsg = "Unexpected error.";
		return false;
	}

	std::shared_ptr<const std::string> filenamePtr = std::make_shared<const std::string>(filename);
	dumpToValue(root, filenamePtr, fullContent.data(), tok.data(), p.toknext, 0);
	return true;
}

std::string cfg::JsonParser::getExtendedErrorMsg() const
{
	return mFilename + ":" + std::to_string(mLineNumber) + ": " + mErrorMsg;
}

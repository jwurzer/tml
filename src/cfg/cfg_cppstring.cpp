#include <cfg/cfg_cppstring.h>
#include <cfg/cfg.h>
#include <sstream>

namespace {
	void addTab(std::stringstream& ssout, unsigned int count)
	{
		switch (count) {
			case 0:
				break;
			case 1:
				ssout << "\t";
				break;
			case 2:
				ssout << "\t\t";
				break;
			case 3:
				ssout << "\t\t\t";
				break;
			case 4:
				ssout << "\t\t\t\t";
				break;
			case 5:
				ssout << "\t\t\t\t\t";
				break;
			default:
				for (unsigned int i = 0; i < count; i++) {
					ssout << '\t';
				}
				break;
		}
	}
}
std::string cfg::cppstring::valueToString(unsigned int deep,
		const Value &cfgValue, bool addFormatArguments, bool addIndentation)
{
	std::string ns = "cfg";
	std::stringstream ss;
	bool addVector = false;
	if (addIndentation) {
		addTab(ss, deep);
	}
	if (!ns.empty()) {
		ss << ns << "::";
	}
	switch (cfgValue.mType) {
		case Value::TYPE_NONE:
			ss << "none(";
			break;
		case Value::TYPE_NULL:
			ss << "nullValue(";
			break;
		case Value::TYPE_BOOL:
			ss << "boolValue(" << (cfgValue.mBool ? "true" : "false");
			break;
		case Value::TYPE_FLOAT:
			ss << "floatValue(" << cfgValue.mFloatingPoint;
			break;
		case Value::TYPE_INT:
			ss << "intValue(" << cfgValue.mInteger;
			if (cfgValue.mParseBase != 10) {
				ss << ", " << cfgValue.mParseBase;
			}
			break;
		case Value::TYPE_TEXT:
			// TODO escape special characters
			ss << "text(\"" << cfgValue.mText << "\"";
			break;
		case Value::TYPE_COMMENT:
			ss << "commentValue(\"" << cfgValue.mText << "\"";
			break;
		case Value::TYPE_ARRAY: {
			ss << "array(";
			if (!cfgValue.mArray.empty()) {
				if (addVector) {
					ss << "std::vector<";
					if (!ns.empty()) {
						ss << ns << "::";
					}
					ss << "Value>{\n";
				}
				else {
					ss << "{\n";
				}
				int i = 0;
				for (const auto &cfgvalue : cfgValue.mArray) {
					if (i != 0) {
						ss << ",\n";
					}
					ss << valueToString(deep + 1, cfgvalue, addFormatArguments);
					++i;
				}
				ss << "}";
			}
			break;
		}
		case Value::TYPE_OBJECT:
			ss << "object(";
			if (!cfgValue.mObject.empty()) {
				if (addVector) {
					ss << "std::vector<";
					if (!ns.empty()) {
						ss << ns << "::";
					}
					ss << "NameValuePair>{\n";
				}
				else {
					ss << "{\n";
				}
				int i = 0;
				for (const auto& cfgpair: cfgValue.mObject) {
					if (i != 0) {
						ss << ",\n";
					}
					ss << nameValuePairToString(deep + 1, cfgpair, addFormatArguments);
					++i;
				}
				ss << "}";
			}
			break;
	}

	ss << ")";
	return ss.str();
}

std::string cfg::cppstring::nameValuePairToString(unsigned int deep,
		const NameValuePair &cfgPair, bool addFormatArguments)
{
	std::string ns = "cfg";
	std::stringstream ss;
	addTab(ss, deep);
	if (!ns.empty()) {
		ss << ns << "::";
	}
	if (cfgPair.isEmpty()) {
		ss << "empty()";
	}
	else if (cfgPair.isComment()) {
		const Value& comment = cfgPair.mName;
		ss << "comment(\"" << comment.mText << "\")";
	}
	else {
		bool multipleLines =
				(cfgPair.mName.isArray() || cfgPair.mName.isObject() ||
				cfgPair.mValue.isArray() || cfgPair.mValue.isObject());
		if (cfgPair.mValue.isEmpty()) {
			ss << "single(";
		}
		else {
			ss << "pair(";
		}
		if (multipleLines) {
			ss << "\n";
		}
		ss << valueToString(deep + 1, cfgPair.mName, addFormatArguments, multipleLines);
		if (!cfgPair.mValue.isEmpty()) {
			ss << "," << (multipleLines ? "\n" : " ");
			ss << valueToString(deep + 1, cfgPair.mValue, addFormatArguments, multipleLines);
		}
		if (addFormatArguments && cfgPair.mDeep != -1) {
			ss << "," << (multipleLines ? "\n" : " ");
			addTab(ss, deep + 1);
			ss << cfgPair.mDeep << ")";
		}
		else {
			ss << ")";
		}
	}
	return ss.str();
}


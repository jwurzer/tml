#include <cfg/cfg_string.h>
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
std::string cfg::cfgstring::valueToString(unsigned int deep,
		const Value &cfgValue, const std::string &name)
{
	const char* strType[] = {
			"TYPE_NONE",
			"TYPE_NULL",
			"TYPE_BOOL",
			"TYPE_FLOAT",
			"TYPE_INT",
			"TYPE_TEXT",
			"TYPE_COMMENT",
			"TYPE_ARRAY",
			"TYPE_OBJECT",

			"error"
	};

	std::stringstream ss;
	addTab(ss, deep);
	if (!name.empty()) {
		ss << name << ": ";
	}
	ss << "VALUE" << cfgValue.getFilePosition() << ": {";
	//addTab(ss, deep + 1);
	ss << "NVP-DEEP: " << cfgValue.mNvpDeep <<
			", TYPE: " << strType[(cfgValue.mType < 8) ? cfgValue.mType : 8] << "(" << cfgValue.mType << ")";
	bool newline = false;
	switch (cfgValue.mType) {
		case Value::TYPE_NONE:
			break;
		case Value::TYPE_NULL:
			ss << ", null";
			break;
		case Value::TYPE_BOOL:
			ss << ", BOOL: " << (cfgValue.mBool ? "true" : "false");
			break;
		case Value::TYPE_FLOAT:
			ss << ", FLOAT: " << cfgValue.mFloatingPoint;
			break;
		case Value::TYPE_INT:
			ss << ", PARSE BASE: " << cfgValue.mParseBase;
			ss << ", INT: " << cfgValue.mInteger;
			break;
		case Value::TYPE_TEXT:
			ss << ", TEXT: \"" << cfgValue.mText << "\"";
			break;
		case Value::TYPE_COMMENT:
			ss << ", TEXT: \"" << cfgValue.mText << "\"";
			break;
		case Value::TYPE_ARRAY: {
			ss << ", SIZE: " << cfgValue.mArray.size() << ",\n";
			int i = 0;
			for (const auto &cfgvalue : cfgValue.mArray) {
				ss << valueToString(deep + 1, cfgvalue,
						"index " + std::to_string(i));
				++i;
			}
			newline = true;
			break;
		}
		case Value::TYPE_OBJECT:
			ss << ", SIZE: " << cfgValue.mObject.size() << ",\n";
			int i = 0;
			for (const auto& cfgpair : cfgValue.mObject) {
				ss << nameValuePairToString(deep + 1, cfgpair,
						"obj attr " + std::to_string(i));
				++i;
			}
			newline = true;
			break;
	}

	if (newline) {
		addTab(ss, deep);
	}
	ss << "}\n";
	return ss.str();
}

std::string cfg::cfgstring::nameValuePairToString(unsigned int deep,
		const NameValuePair &cfgPair, const std::string &name)
{
	std::stringstream ss;
	addTab(ss, deep);
	if (!name.empty()) {
		ss << name << ": ";
	}
	ss << "VALUE PAIR: {DEEP: " << cfgPair.mDeep << ",\n";
	ss << valueToString(deep + 1, cfgPair.mName, "name");
	ss << valueToString(deep + 1, cfgPair.mValue, "value");
	addTab(ss, deep);
	ss << "}\n";
	return ss.str();
}


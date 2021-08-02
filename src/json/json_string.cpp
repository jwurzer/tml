#include <json/json_string.h>
#include <cfg/cfg.h>
#include <sstream>

namespace cfg
{
	namespace
	{
		void addCharAsIndent(std::stringstream& ssout, unsigned int count, char ch)
		{
			char indentStr[6] = {ch, ch, ch, ch, ch, 0};
			if (count <= 5) {
				indentStr[count] = 0;
				ssout << indentStr;
				return;
			}
			for (unsigned int i = 0; i < count; i++) {
				ssout << ch;
			}
		}

		void addIndent(std::stringstream& ssout, unsigned int count, int intentMode)
		{
			if (intentMode < 0) {
				return;
			}
			if (intentMode == 0) {
				addCharAsIndent(ssout, count, '\t');
				return;
			}
			// --> intentMode is > 0
			addCharAsIndent(ssout, count * static_cast<unsigned int>(intentMode), ' ');
		}

		void addNewline(std::stringstream& ssout, int indentMode)
		{
			if (indentMode >= -1) {
				ssout << '\n';
			}
		}

		void addTextForJson(std::stringstream& ss, const std::string& text)
		{
			if (text.empty()) {
				ss << "\"\"";
				return;
			}
			std::size_t len = text.length();
			ss << "\"";
			for (std::size_t i = 0; i < len; ++i) {
				char ch = text[i];
				switch (ch) {
					case '\t':
						ss << "\\t";
						break;
					case '\n':
						ss << "\\n";
						break;
					case '\\':
						ss << "\\\\";
						break;
					case '"':
						ss << "\\\"";
						break;
					default:
						ss << ch;
						break;
				}
			}
			ss << "\"";
		}

		void addSimpleValueToStringStream(const Value& cfgValue,
				std::stringstream& ss)
		{
			switch (cfgValue.mType) {
				case Value::TYPE_NULL:
					ss << "null";
					break;
				case Value::TYPE_BOOL:
					ss << (cfgValue.mBool ? "true" : "false");
					break;
				case Value::TYPE_FLOAT:
					ss << cfgValue.mFloatingPoint;
					break;
				case Value::TYPE_INT:
					ss << cfgValue.mInteger;
					break;
				case Value::TYPE_TEXT:
					addTextForJson(ss, cfgValue.mText);
					break;
				default:
					ss << "ERROR";
					break;
			}
		}

		void addValueToStringStream(unsigned int deep, const Value& cfgValue,
				std::stringstream& ss, int indentMode)
		{
			if (cfgValue.isSimple()) {
				addSimpleValueToStringStream(cfgValue, ss);
				return;
			}
			if (cfgValue.isArray()) {
				std::size_t cnt = cfgValue.mArray.size();
				bool fullArrayIsSimple = true;
				for (std::size_t i = 0; i < cnt; ++i) {
					if (!cfgValue.mArray[i].isSimple()) {
						fullArrayIsSimple = false;
						break;
					}
				}
				if (fullArrayIsSimple) {
					ss << "[";
					for (std::size_t i = 0; i < cnt; ++i) {
						addSimpleValueToStringStream(cfgValue.mArray[i], ss);
						if (i + 1 < cnt) {
							ss << ", ";
						}
					}
					ss << "]";
				}
				else {
					ss << "[";
					addNewline(ss, indentMode);
					for (std::size_t i = 0; i < cnt; ++i) {
						addIndent(ss, deep + 1, indentMode);
						if (cfgValue.mArray[i].isSimple()) {
							addSimpleValueToStringStream(cfgValue.mArray[i], ss);
						}
						else {
							::cfg::jsonstring::valueToStringStream(deep + 1, cfgValue.mArray[i], ss, indentMode);
						}
						if (i + 1 < cnt) {
							ss << ",";
						}
						addNewline(ss, indentMode);
					}
					addIndent(ss, deep, indentMode);
					ss << "]";
				}
				return;
			}
			if (cfgValue.isEmpty()) {
				ss << "\"(empty)\"";
			}
			ss << "ERROR";
		}

		void addObjectToStringStream(unsigned int deep,
				const Value &cfgValue, std::stringstream& ss, int indentMode)
		{
			//addIndent(ss, deep, indentMode);
			if (cfgValue.mObject.empty()) {
				ss << "{}";
				return;
			}
			ss << "{";
			addNewline(ss, indentMode);
			std::size_t cfgPairCnt = cfgValue.mObject.size();
			for (std::size_t i = 0; i < cfgPairCnt; ++i) {
				const auto& cfgPair = cfgValue.mObject[i];
				jsonstring::nameValuePairToStringStream(deep + 1, cfgPair, ss,
						indentMode);
				if (i + 1 < cfgPairCnt) {
					ss << ",";
				}
				addNewline(ss, indentMode);
			}
			addIndent(ss, deep, indentMode);
			ss << "}";
		}
	}
}

void cfg::jsonstring::valueToStringStream(unsigned int deep,
		const Value &cfgValue, std::stringstream& ss, int indentMode,
		bool addStartingIndent, bool addEndingNewline)
{
	if (cfgValue.isObject()) {
		addObjectToStringStream(deep, cfgValue, ss, indentMode);
	}
	else {
		if (addStartingIndent) {
			addIndent(ss, deep, indentMode);
		}
		addValueToStringStream(deep, cfgValue, ss, indentMode);
		if (addEndingNewline) {
			addNewline(ss, indentMode);
		}
	}
}

std::string cfg::jsonstring::valueToString(unsigned int deep,
		const Value& cfgValue, int indentMode)
{
	std::stringstream ss;
	valueToStringStream(deep, cfgValue, ss, indentMode);
	return ss.str();
}

void cfg::jsonstring::nameValuePairToStringStream(unsigned int deep,
		const NameValuePair& cfgPair, std::stringstream& ss, int indentMode)
{
	if (cfgPair.mName.isEmpty() && cfgPair.mValue.isEmpty()) {
		// --> empty line but JSON doesn't support empty lines
		// --> add "(empty)": "(empty)" for empty line.
		addIndent(ss, deep, indentMode);
		ss << "\"(empty)\": \"(empty)\"";
		return;
	}
	if (cfgPair.mName.isComment() && cfgPair.mValue.isEmpty()) {
		// --> comment but JSON doesn't support comments
		// --> add "(comment)": "<comment-text>" for comment.
		addIndent(ss, deep, indentMode);
		ss << "\"(comment)\": \"" << cfgPair.mName.mText << "\"";
		return;
	}
	addIndent(ss, deep, indentMode);
	if (cfgPair.mName.isText()) {
		addValueToStringStream(deep, cfgPair.mName, ss, indentMode);
	}
	else {
		ss << "\"(name)\": ";
		cfg::jsonstring::valueToStringStream(deep, cfgPair.mName, ss, indentMode, false, false);
		if (cfgPair.mValue.isEmpty()) {
			// in this case no "(empty)" must be added because the value
			// is used for the name.
			return;
		}
		ss << ",";
		addNewline(ss, indentMode);
		addIndent(ss, deep, indentMode);
		ss << "\"(value)\"";
	}
	if (cfgPair.mValue.isObject()) {
		ss << ": ";
		addObjectToStringStream(deep, cfgPair.mValue, ss, indentMode);
	}
	else if (cfgPair.mValue.isEmpty()) {
		ss << ": \"(empty)\"";
	}
	else {
		ss << ": ";
		addValueToStringStream(deep, cfgPair.mValue, ss, indentMode);
	}
}

std::string cfg::jsonstring::nameValuePairToString(unsigned int deep,
		const NameValuePair &cfgPair, int indentMode)
{
	std::stringstream ss;
	nameValuePairToStringStream(deep, cfgPair, ss, indentMode);
	return ss.str();
}

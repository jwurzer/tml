#include <tml/tml_string.h>
#include <cfg/cfg.h>
#include <sstream>

namespace cfg
{
	namespace
	{
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

		std::string getTextForTml(const std::string& text)
		{
			if (text.empty()) {
				return "\"\"";
			}
			std::size_t len = text.length();
			bool mustBeEscaped = false;
			for (std::size_t i = 0; i < len; ++i) {
				char ch = text[i];
				if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\\' || ch == '"') {
					mustBeEscaped = true;
					break;
				}
			}
			if (!mustBeEscaped && text != "true" && text != "false" && text != "null") {
				return text;
			}
			std::stringstream ss;
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
			return ss.str();
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
					ss << getTextForTml(cfgValue.mText);
					break;
				default:
					ss << "ERROR";
					break;
			}
		}

		void addValueToStringStream(const Value& cfgValue,
				std::stringstream& ss)
		{
			if (cfgValue.isSimple()) {
				addSimpleValueToStringStream(cfgValue, ss);
				return;
			}
			if (cfgValue.isArray()) {
				std::size_t cnt = cfgValue.mArray.size();
				for (std::size_t i = 0; i < cnt; ++i) {
					addSimpleValueToStringStream(cfgValue.mArray[i], ss);
					if (i + 1 < cnt) {
						ss << " ";
					}
				}
				return;
			}
			ss << "ERROR";
		}

		void addObjectToStringStream(unsigned int deep,
				const Value &cfgValue, std::stringstream& ss)
		{
			for (const auto& cfgPair : cfgValue.mObject) {
				tmlstring::nameValuePairToStringStream(deep, cfgPair, ss);
			}
		}
	}
}

void cfg::tmlstring::valueToStringStream(unsigned int deep,
		const Value &cfgValue, std::stringstream& ss)
{
	if (cfgValue.isObject()) {
		addObjectToStringStream(deep, cfgValue, ss);
	}
	else {
		addTab(ss, deep);
		addValueToStringStream(cfgValue, ss);
		ss << "\n";
	}
}

std::string cfg::tmlstring::valueToString(unsigned int deep,
		const Value& cfgValue)
{
	std::stringstream ss;
	valueToStringStream(deep, cfgValue, ss);
	return ss.str();
}

void cfg::tmlstring::nameValuePairToStringStream(unsigned int deep,
		const NameValuePair& cfgPair, std::stringstream& ss)
{
	if (cfgPair.mName.isEmpty() && cfgPair.mValue.isEmpty()) {
		if (cfgPair.mDeep >= 0) {
			addTab(ss, cfgPair.mDeep);
		}
		ss << "\n";
		return;
	}
	if (cfgPair.mName.isComment() && cfgPair.mValue.isEmpty()) {
		if (cfgPair.mDeep >= 0) {
			addTab(ss, cfgPair.mDeep);
		}
		else {
			addTab(ss, deep);
		}
		ss << "#" << cfgPair.mName.mText << "\n";
		return;
	}
	if (!cfgPair.mName.isSimple()) {
		ss << "name can be only a simple value";
		return;
	}
	addTab(ss, deep);
	addValueToStringStream(cfgPair.mName, ss);
	if (cfgPair.mValue.isObject()) {
		ss << "\n";
		addObjectToStringStream(deep + 1, cfgPair.mValue, ss);
	}
	else if (cfgPair.mValue.isEmpty()) {
		ss << "\n";
	}
	else {
		ss << " = ";
		addValueToStringStream(cfgPair.mValue, ss);
		ss << "\n";
	}
}

std::string cfg::tmlstring::nameValuePairToString(unsigned int deep,
		const NameValuePair &cfgPair)
{
	std::stringstream ss;
	nameValuePairToStringStream(deep, cfgPair, ss);
	return ss.str();
}

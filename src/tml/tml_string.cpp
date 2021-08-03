#include <tml/tml_string.h>
#include <cfg/cfg.h>
#include <sstream>

namespace cfg
{
	namespace
	{
		void addTab(std::stringstream& ssout, int count)
		{
			if (count < 0) {
				return;
			}
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
					for (int i = 0; i < count; i++) {
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

		void addValueToStringStream(unsigned int deep, const Value& cfgValue,
				std::stringstream& ss)
		{
			if (cfgValue.isSimple()) {
				addSimpleValueToStringStream(cfgValue, ss);
				return;
			}
			if (cfgValue.isArray()) {
				if (cfgValue.mArray.empty()) {
					ss << "[]";
					return;
				}
				// TODO check if not empty if its only comments or empty lines...
				std::size_t cnt = cfgValue.mArray.size();
				bool fullArrayIsSimple = true;
				for (std::size_t i = 0; i < cnt; ++i) {
					if (!cfgValue.mArray[i].isSimple()) {
						fullArrayIsSimple = false;
						break;
					}
				}
				if (fullArrayIsSimple) {
					for (std::size_t i = 0; i < cnt; ++i) {
						addSimpleValueToStringStream(cfgValue.mArray[i], ss);
						if (i + 1 < cnt) {
							ss << " ";
						}
					}
				}
				else {
					ss << "[]\n";
					for (std::size_t i = 0; i < cnt; ++i) {
						bool forceDeepByStoredDeepValue = false; // TODO remove
						int storedDeep = -2; // TODO remove
						if (cfgValue.mArray[i].isObject()) {
							addTab(ss, deep + 1);
							ss << "{}\n";
							::cfg::tmlstring::valueToStringStream(deep + 2,
									cfgValue.mArray[i], ss,
									forceDeepByStoredDeepValue, storedDeep);
						}
						else {
							addTab(ss, deep + 1);
							addValueToStringStream(deep + 1, cfgValue.mArray[i], ss);
							if (i + 1 < cnt) {
								ss << "\n";
							}
						}
					}
				}
				return;
			}
			ss << "ERROR";
		}

		void addObjectToStringStream(unsigned int deep,
				const Value &cfgValue, std::stringstream& ss,
				bool forceDeepByStoredDeepValue)
		{
			for (const auto& cfgPair : cfgValue.mObject) {
				tmlstring::nameValuePairToStringStream(deep, cfgPair, ss,
						forceDeepByStoredDeepValue);
			}
		}
	}
}

void cfg::tmlstring::valueToStringStream(unsigned int deep,
		const Value &cfgValue, std::stringstream& ss,
		bool forceDeepByStoredDeepValue, int storedDeep)
{
	if (storedDeep <= -2) {
		storedDeep = deep;
	}
	if (cfgValue.isObject()) {
		addObjectToStringStream(deep, cfgValue, ss, forceDeepByStoredDeepValue);
	}
	else {
		addTab(ss, forceDeepByStoredDeepValue ? storedDeep : deep);
		addValueToStringStream(deep, cfgValue, ss);
		ss << "\n";
	}
}

std::string cfg::tmlstring::valueToString(unsigned int deep,
		const Value& cfgValue, bool forceDeepByStoredDeepValue, int storedDeep)
{
	std::stringstream ss;
	valueToStringStream(deep, cfgValue, ss, forceDeepByStoredDeepValue, storedDeep);
	return ss.str();
}

void cfg::tmlstring::nameValuePairToStringStream(unsigned int deep,
		const NameValuePair& cfgPair, std::stringstream& ss,
		bool forceDeepByStoredDeepValue)
{
	if (cfgPair.mName.isEmpty() && cfgPair.mValue.isEmpty()) {
		addTab(ss, cfgPair.mDeep); // if mDeep is negative then its ignored
		ss << "\n";
		return;
	}
	if (cfgPair.mName.isComment() && cfgPair.mValue.isEmpty()) {
		if (cfgPair.mDeep >= 0 || forceDeepByStoredDeepValue) {
			// if mDeep is negative (can only be the case if
			// forceDeepByStoredDeepValue is true) then its ignored
			addTab(ss, cfgPair.mDeep);
		}
		else {
			addTab(ss, deep);
		}
		ss << "#" << cfgPair.mName.mText << "\n";
		return;
	}
	if (cfgPair.mName.isObject()) {
		ss << "name can't be an object\n";
		return;
	}
	addTab(ss, forceDeepByStoredDeepValue ? cfgPair.mDeep : deep);
	addValueToStringStream(deep, cfgPair.mName, ss);
	if (cfgPair.mValue.isObject()) {
		if (cfgPair.mValue.mObject.empty()) {
			// TODO check if not empty if its only comments or empty lines...
			ss << " = {}\n";
		}
		else {
			ss << "\n";
			addObjectToStringStream(deep + 1, cfgPair.mValue, ss,
					forceDeepByStoredDeepValue);
		}
	}
	else if (cfgPair.mValue.isEmpty()) {
		ss << "\n";
	}
	else {
		ss << " = ";
		addValueToStringStream(deep, cfgPair.mValue, ss);
		ss << "\n";
	}
}

std::string cfg::tmlstring::nameValuePairToString(unsigned int deep,
		const NameValuePair &cfgPair,
		bool forceDeepByStoredDeepValue)
{
	std::stringstream ss;
	nameValuePairToStringStream(deep, cfgPair, ss, forceDeepByStoredDeepValue);
	return ss.str();
}

#include <tml/tml_string.h>
#include <cfg/cfg.h>
#include <sstream>

namespace cfg
{
	namespace
	{
		void addTab(std::ostream& ssout, int count)
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

		std::string getTextForTml(const std::string& text,
				bool forceTextWithQuotes)
		{
			if (text.empty()) {
				return "\"\"";
			}
			std::size_t len = text.length();
			bool mustBeEscaped = false;
			if (forceTextWithQuotes) {
				mustBeEscaped = true;
			}
			else {
				bool isNumber = true;
				bool numberWithFloatingPoint = false;
				for (std::size_t i = 0; i < len; ++i) {
					char ch = text[i];
					if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\\' || ch == '"') {
						mustBeEscaped = true;
						break;
					}
					if (isNumber) {
						if ((i == 0 && (ch == '+' || ch == '-')) ||
								(ch >= '0' && ch <= '9')) {
							continue;
						}
						if (ch == '.') {
							if (!numberWithFloatingPoint) {
								// --> first dot --> can be a floating point number
								numberWithFloatingPoint = true;
							}
							else {
								// --> second dot --> can be a number
								isNumber = false;
								numberWithFloatingPoint = false;
							}
							continue;
						}
						// --> can't be a number
						isNumber = false;
					}
				}
				if (isNumber) {
					mustBeEscaped = true;
				}
			}
			if (!mustBeEscaped && text != "true" && text != "false"
					&& text != "null" && text != "[]" && text != "{}") {
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

		void addSimpleValueToStream(const Value& cfgValue,
				std::ostream& ss)
		{
			switch (cfgValue.mType) {
				case Value::TYPE_NULL:
					ss << "null";
					break;
				case Value::TYPE_BOOL:
					ss << (cfgValue.mBool ? "true" : "false");
					break;
				case Value::TYPE_FLOAT: {
					// also a dot is used! otherwise it will be interpreted as int instead of a float.
					std::stringstream tmpSs;
					tmpSs.imbue(std::locale()); // --> always use a dot and no comma etc.
					tmpSs << cfgValue.mFloatingPoint;
					std::string fp = tmpSs.str();
					bool dotIncluded = false;
					for (char c : fp) {
						if (c == '.') {
							dotIncluded = true;
						}
					}
					ss << fp;
					if (!dotIncluded) {
						ss << ".0";
					}
					break;
				}
				case Value::TYPE_INT:
					ss << cfgValue.mInteger;
					break;
				case Value::TYPE_TEXT:
					ss << getTextForTml(cfgValue.mText, cfgValue.mParseTextWithQuotes);
					break;
				default:
					ss << "ERROR";
					break;
			}
		}

		void addObjectToStream(unsigned int deep,
				const Value &cfgValue, std::ostream& ss,
				bool forceDeepByStoredDeepValue);

		void addValueToStream(unsigned int deep, const Value& cfgValue,
				std::ostream& ss, bool forceDeepByStoredDeepValue)
		{
			if (cfgValue.isEmpty()) {
				return;
			}
			if (cfgValue.isComment()) {
				ss << "#" << cfgValue.mText;
				return;
			}
			if (cfgValue.isSimple()) {
				addSimpleValueToStream(cfgValue, ss);
				return;
			}
			if (cfgValue.isArray()) {
				if (cfgValue.mArray.empty()) {
					ss << "[]";
					return;
				}
				// TODO check if not empty if its only comments or empty lines...
				std::size_t cnt = cfgValue.mArray.size();
				bool fullArrayIsSimple = !cfgValue.isComplexArray();
				if (fullArrayIsSimple) {
					for (std::size_t i = 0; i < cnt; ++i) {
						addSimpleValueToStream(cfgValue.mArray[i], ss);
						if (i + 1 < cnt) {
							ss << " ";
						}
					}
				}
				else {
					ss << "[]\n";
					for (std::size_t i = 0; i < cnt; ++i) {
						addTab(ss, deep + 1);
						addValueToStream(deep + 1, cfgValue.mArray[i], ss, forceDeepByStoredDeepValue);
						if (i + 1 < cnt) {
							ss << "\n";
						}
					}
				}
				return;
			}
			if (cfgValue.isObject()) {
				ss << "{}";
				if (!cfgValue.mObject.empty()) {
					ss << "\n";
					addObjectToStream(deep + 1, cfgValue, ss, forceDeepByStoredDeepValue);
				}
				return;
			}
			ss << "ERROR";
		}

		void addNameValuePairToStream(unsigned int deep,
				const NameValuePair& cfgPair, std::ostream& ss,
				bool forceDeepByStoredDeepValue);

		void addObjectToStream(unsigned int deep,
				const Value &cfgValue, std::ostream& ss,
				bool forceDeepByStoredDeepValue)
		{
			std::size_t cnt = cfgValue.mObject.size();
			for (std::size_t i = 0; i < cnt; ++i) {
				const auto& cfgPair = cfgValue.mObject[i];
				addNameValuePairToStream(deep, cfgPair, ss,
						forceDeepByStoredDeepValue);
				if (i + 1 < cnt) {
					ss << "\n";
				}
			}
		}

		void addNameValuePairToStream(unsigned int deep,
				const NameValuePair& cfgPair, std::ostream& ss,
				bool forceDeepByStoredDeepValue)
		{
			if (cfgPair.mName.isEmpty() && cfgPair.mValue.isEmpty()) {
				addTab(ss, cfgPair.mDeep); // if mDeep is negative then its ignored
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
				ss << "#" << cfgPair.mName.mText;
				return;
			}
			if (cfgPair.mName.isObject()) {
				ss << "name can't be an object";
				return;
			}
			addTab(ss, forceDeepByStoredDeepValue ? cfgPair.mDeep : deep);
			addValueToStream(deep, cfgPair.mName, ss, forceDeepByStoredDeepValue);
			if (cfgPair.mValue.isObject()) {
				if (cfgPair.mValue.mObject.empty()) {
					// TODO check if not empty if its only comments or empty lines...
					ss << " = {}";
				}
				else {
					ss << "\n";
					addObjectToStream(deep + 1, cfgPair.mValue, ss, forceDeepByStoredDeepValue);
				}
			}
			else if (cfgPair.mValue.isEmpty()) {
				// nothing to do
			}
			else {
				ss << " = ";
				addValueToStream(deep, cfgPair.mValue, ss,
						forceDeepByStoredDeepValue);
			}
		}
	}
}

void cfg::tmlstring::valueToStream(unsigned int deep,
		const Value &cfgValue, std::ostream& ss,
		bool forceDeepByStoredDeepValue, int storedDeep)
{
	if (storedDeep <= -2) {
		storedDeep = deep;
	}
	if (cfgValue.isObject()) {
		addObjectToStream(deep, cfgValue, ss, forceDeepByStoredDeepValue);
		ss << "\n";
	}
	else {
		addTab(ss, forceDeepByStoredDeepValue ? storedDeep : deep);
		addValueToStream(deep, cfgValue, ss, forceDeepByStoredDeepValue);
		ss << "\n";
	}
}

std::string cfg::tmlstring::valueToString(unsigned int deep,
		const Value& cfgValue, bool forceDeepByStoredDeepValue, int storedDeep)
{
	std::stringstream ss;
	valueToStream(deep, cfgValue, ss, forceDeepByStoredDeepValue, storedDeep);
	return ss.str();
}

std::string cfg::tmlstring::plainValueToString(const Value& cfgValue)
{
	if (cfgValue.isEmpty()) {
		return "";
	}
	std::stringstream ss;
	if (cfgValue.isComment()) {
		ss << "#" << cfgValue.mText;
		return ss.str();
	}
	if (cfgValue.isArray()) {
		if (cfgValue.isComplexArray()) {
			return "ERROR";
		}
		std::size_t cnt = cfgValue.mArray.size();
		if (!cnt) {
			// empty array
			return "[]";
		}
		for (std::size_t i = 0; i < cnt; ++i) {
			addSimpleValueToStream(cfgValue.mArray[i], ss);
			if (i + 1 < cnt) {
				ss << " ";
			}
		}
		return ss.str();
	}
	if (cfgValue.isObject()) {
		if (!cfgValue.mObject.empty()) {
			ss << "ERROR";
		}
		ss << "{}";
		return ss.str();
	}

	addSimpleValueToStream(cfgValue, ss);
	return ss.str();
}

void cfg::tmlstring::nameValuePairToStream(unsigned int deep,
		const NameValuePair& cfgPair, std::ostream& ss,
		bool forceDeepByStoredDeepValue)
{
	addNameValuePairToStream(deep, cfgPair, ss, forceDeepByStoredDeepValue);
	ss << "\n";
}

std::string cfg::tmlstring::nameValuePairToString(unsigned int deep,
		const NameValuePair &cfgPair,
		bool forceDeepByStoredDeepValue)
{
	std::stringstream ss;
	nameValuePairToStream(deep, cfgPair, ss, forceDeepByStoredDeepValue);
	return ss.str();
}

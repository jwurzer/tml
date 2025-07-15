#include <tml/tml_string.h>
#include <cfg/cfg.h>
#include <sstream>

namespace cfg
{
	namespace
	{
		void addTmlLine(TmlLines& tl, int deep, const std::string& line)
		{
			tl.mLines.emplace_back(deep, line);
		}

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

		// ss is allowed to be null
		void addObjectToStream(unsigned int deep,
				const Value &cfgValue, std::ostream* ss, TmlLines* tl,
				bool forceDeepByStoredDeepValue);

		// ss is allowed to be null
		void addValueToStream(unsigned int deep, const Value& cfgValue,
				std::ostream* ss, TmlLines* tl, bool forceDeepByStoredDeepValue)
		{
			if (cfgValue.isEmpty()) {
				if (tl) {
					addTmlLine(*tl, deep, "");
				}
				return;
			}
			if (cfgValue.isComment()) {
				if (ss) {
					*ss << "#" << cfgValue.mText;
				}
				if (tl) {
					addTmlLine(*tl, deep, "#" + cfgValue.mText);
				}
				return;
			}
			if (cfgValue.isSimple()) {
				if (ss) {
					addSimpleValueToStream(cfgValue, *ss);
				}
				if (tl) {
					std::stringstream ssTl;
					addSimpleValueToStream(cfgValue, ssTl);
					addTmlLine(*tl, deep, ssTl.str());
				}
				return;
			}
			if (cfgValue.isArray()) {
				if (cfgValue.mArray.empty()) {
					if (ss) {
						*ss << "[]";
					}
					if (tl) {
						addTmlLine(*tl, deep, "[]");
					}
					return;
				}
				// TODO check if not empty if its only comments or empty lines...
				std::size_t cnt = cfgValue.mArray.size();
				bool fullArrayIsSimple = !cfgValue.isComplexArray();
				if (fullArrayIsSimple) {
					std::unique_ptr<std::stringstream> ssTl;
					if (tl) {
						//ssTl = std::make_unique<std::stringstream>();
						ssTl = std::unique_ptr<std::stringstream>(new std::stringstream);
					}
					for (std::size_t i = 0; i < cnt; ++i) {
						if (ss) {
							addSimpleValueToStream(cfgValue.mArray[i], *ss);
							if (i + 1 < cnt) {
								*ss << " ";
							}
						}
						if (tl) {
							addSimpleValueToStream(cfgValue.mArray[i], *ssTl);
							if (i + 1 < cnt) {
								*ssTl << " ";
							}
						}
					}
					if (tl) {
						addTmlLine(*tl, deep, ssTl->str());
					}
				}
				else {
					if (ss) {
						*ss << "[]\n";
					}
					TmlLines* tlSub = nullptr;
					if (tl) {
						addTmlLine(*tl, deep, "[]");
						//tl->mLines.back().mSubLines = std::make_unique<TmlLines>();
						tl->mLines.back().mSubLines = std::unique_ptr<TmlLines>(new TmlLines);
						tlSub = tl->mLines.back().mSubLines.get();
					}
					for (std::size_t i = 0; i < cnt; ++i) {
						if (ss) {
							addTab(*ss, deep + 1);
						}
						addValueToStream(deep + 1, cfgValue.mArray[i], ss, tlSub, forceDeepByStoredDeepValue);
						if (ss) {
							if (i + 1 < cnt) {
								*ss << "\n";
							}
						}
					}
				}
				return;
			}
			if (cfgValue.isObject()) {
				if (ss) {
					*ss << "{}";
				}
				if (tl) {
					addTmlLine(*tl, deep, "{}");
				}
				if (!cfgValue.mObject.empty()) {
					if (ss) {
						*ss << "\n";
					}
					TmlLines* tlSub = nullptr;
					if (tl) {
						//tl->mLines.back().mSubLines = std::make_unique<TmlLines>();
						tl->mLines.back().mSubLines = std::unique_ptr<TmlLines>(new TmlLines);
						tlSub = tl->mLines.back().mSubLines.get();
					}
					addObjectToStream(deep + 1, cfgValue, ss, tlSub, forceDeepByStoredDeepValue);
				}
				return;
			}
			if (ss) {
				*ss << "ERROR";
			}
			if (tl) {
				addTmlLine(*tl, deep, "ERROR");
			}
		}

		// ss is allowed to be null
		void addNameValuePairToStream(unsigned int deep,
				const NameValuePair& cfgPair, std::ostream* ss, TmlLines* tl,
				bool forceDeepByStoredDeepValue);

		// ss is allowed to be null
		void addObjectToStream(unsigned int deep,
				const Value &cfgValue, std::ostream* ss, TmlLines* tl,
				bool forceDeepByStoredDeepValue)
		{
			std::size_t cnt = cfgValue.mObject.size();
			for (std::size_t i = 0; i < cnt; ++i) {
				const auto& cfgPair = cfgValue.mObject[i];
				addNameValuePairToStream(deep, cfgPair, ss, tl,
						forceDeepByStoredDeepValue);
				if (i + 1 < cnt) {
					if (ss) {
						*ss << "\n";
					}
				}
			}
		}

		// ss is allowed to be null
		void addNameValuePairToStream(unsigned int deep,
				const NameValuePair& cfgPair, std::ostream* ss, TmlLines* tl,
				bool forceDeepByStoredDeepValue)
		{
			if (cfgPair.mName.isEmpty() && cfgPair.mValue.isEmpty()) {
				if (ss) {
					addTab(*ss, cfgPair.mDeep); // if mDeep is negative then its ignored
				}
				if (tl) {
					addTmlLine(*tl, cfgPair.mDeep, "");
				}
				return;
			}
			if (cfgPair.mName.isComment() && cfgPair.mValue.isEmpty()) {
				// if mDeep is negative (can only be the case if
				// forceDeepByStoredDeepValue is true) then its ignored
				int usedDeep = (cfgPair.mDeep >= 0 || forceDeepByStoredDeepValue) ?
						cfgPair.mDeep : int(deep);
				if (ss) {
					addTab(*ss, usedDeep);
					*ss << "#" << cfgPair.mName.mText;
				}
				if (tl) {
					addTmlLine(*tl, usedDeep, "#" + cfgPair.mName.mText);
				}
				return;
			}
			if (cfgPair.mName.isObject()) {
				if (ss) {
					addTab(*ss, forceDeepByStoredDeepValue ? cfgPair.mDeep : deep);
					*ss << "name can't be an object";
				}
				if (tl) {
					addTmlLine(*tl,
							forceDeepByStoredDeepValue ? cfgPair.mDeep : deep,
							"name can't be an object");
				}
				return;
			}
			if (ss) {
				addTab(*ss, forceDeepByStoredDeepValue ? cfgPair.mDeep : deep);
			}
			addValueToStream(deep, cfgPair.mName, ss, tl, forceDeepByStoredDeepValue);
			if (cfgPair.mValue.isObject()) {
				if (cfgPair.mValue.mObject.empty()) {
					// TODO check if not empty if its only comments or empty lines...
					if (ss) {
						*ss << " = {}";
					}
					if (tl) {
						tl->mLines.back().mLine += " = {}";
					}
				}
				else {
					if (ss) {
						*ss << "\n";
					}
					TmlLines* tlSub = nullptr;
					if (tl) {
						//tl->mLines.back().mSubLines = std::make_unique<TmlLines>();
						tl->mLines.back().mSubLines = std::unique_ptr<TmlLines>(new TmlLines);
						tlSub = tl->mLines.back().mSubLines.get();
					}
					addObjectToStream(deep + 1, cfgPair.mValue, ss, tlSub, forceDeepByStoredDeepValue);
				}
			}
			else if (cfgPair.mValue.isEmpty()) {
				// nothing to do
			}
			else {
				if (ss) {
					*ss << " = ";
					addValueToStream(deep, cfgPair.mValue, ss, nullptr,
							forceDeepByStoredDeepValue);
				}
				if (tl) {
					TmlLines tmp;
					addValueToStream(deep, cfgPair.mValue, nullptr, &tmp,
							forceDeepByStoredDeepValue);
					tl->mLines.back().mLine += " = ";
					if (tmp.mLines.size() == 1) {
						tl->mLines.back().mLine += tmp.mLines.back().mLine;
						tl->mLines.back().mSubLines = std::move(tmp.mLines.back().mSubLines);
					}
					else {
						tl->mLines.back().mLine += "ERROR " + std::to_string(tmp.mLines.size()) + " != 1";
					}
				}
			}
		}
	}

	void addToStream(unsigned int deep,
			const Value& cfgValue, std::ostream* ss, TmlLines* tl,
			bool forceDeepByStoredDeepValue, int storedDeep)
	{
		if (storedDeep <= -2) {
			storedDeep = deep;
		}
		if (cfgValue.isObject()) {
			addObjectToStream(deep, cfgValue, ss, tl, forceDeepByStoredDeepValue);
			if (ss) {
				*ss << "\n";
			}
		}
		else {
			if (ss) {
				addTab(*ss, forceDeepByStoredDeepValue ? storedDeep : deep);
			}
			addValueToStream(deep, cfgValue, ss, tl, forceDeepByStoredDeepValue);
			if (ss) {
				*ss << "\n";
			}
		}
	}
}


void cfg::tmlstring::valueToStream(unsigned int deep,
		const Value &cfgValue, std::ostream& ss,
		bool forceDeepByStoredDeepValue, int storedDeep)
{
	addToStream(deep, cfgValue, &ss, nullptr, forceDeepByStoredDeepValue, storedDeep);
}

std::string cfg::tmlstring::valueToString(unsigned int deep,
		const Value& cfgValue, bool forceDeepByStoredDeepValue, int storedDeep)
{
	std::stringstream ss;
	addToStream(deep, cfgValue, &ss, nullptr, forceDeepByStoredDeepValue, storedDeep);
	return ss.str();
}

void cfg::tmlstring::valueToTmlLines(unsigned int deep,
		const Value& cfgValue, TmlLines& tmlLines,
		bool forceDeepByStoredDeepValue, int storedDeep)
{
	addToStream(deep, cfgValue, nullptr, &tmlLines, forceDeepByStoredDeepValue, storedDeep);
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
	addNameValuePairToStream(deep, cfgPair, &ss, nullptr, forceDeepByStoredDeepValue);
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

void cfg::tmlstring::nameValuePairToTmlLines(unsigned int deep,
		const NameValuePair& cfgPair, TmlLines& tmlLines,
		bool forceDeepByStoredDeepValue)
{
	addNameValuePairToStream(deep, cfgPair, nullptr, &tmlLines, forceDeepByStoredDeepValue);
}

void cfg::tmlstring::tmlLinesToStream(const TmlLines& tmlLines, std::ostream& s)
{
	for (const TmlLine& tl : tmlLines.mLines) {
		addTab(s, tl.mDeep);
		s << tl.mLine;
		s << "\n";
		if (tl.mSubLines) {
			tmlLinesToStream(*tl.mSubLines, s);
		}
	}
}

std::string cfg::tmlstring::tmlLinesToString(const TmlLines& tmlLines)
{
	std::stringstream ss;
	tmlLinesToStream(tmlLines, ss);
	return ss.str();
}

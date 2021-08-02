#ifndef CFG_JSON_STRING_H
#define CFG_JSON_STRING_H

#include <cfg/export.h>

#include <string>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace jsonstring
	{
		/**
		 * TODO: currently if the name/value pair tree is not JSON compatible
		 *       then special strings for the name is used. For example
		 *       "(comment)", "(empty)", "(name)" and "(value)". For a empty
		 *       value also "(empty)" is used.
		 *       So the created JSON format should be always JSON format compatible.
		 *       This special modification currently always applied if necessary.
		 *       But if it is allowed should be set by a parameter/option.
		 */
		/**
		 * @param indentMode -2 for indention and no line breaks.
		 *        -1 for new lines but no indention
		 *        0 with tab indention
		 *        >0 with space indention. The indentMode value defines
		 *        how much spaces are used for one indention.
		 */
		CFG_API
		void valueToStringStream(unsigned int deep,
				const Value& cfgValue, std::stringstream& ss,
				int indentMode, bool addStartingIndent = true,
				bool addEndingNewline = true);

		CFG_API
		std::string valueToString(unsigned int deep, const Value& cfgValue,
				int indentMode);

		CFG_API
		void nameValuePairToStringStream(unsigned int deep,
				const NameValuePair& cfgPair, std::stringstream& ss,
				int indentMode);

		CFG_API
		std::string nameValuePairToString(unsigned int deep,
				const NameValuePair& cfgPair,
				int indentMode);
	};
}

#endif

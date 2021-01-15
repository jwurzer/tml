#ifndef CFG_TML_STRING_H
#define CFG_TML_STRING_H

#include <cfg/export.h>

#include <string>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace tmlstring
	{
		CFG_API
		void valueToStringStream(unsigned int deep,
				const Value& cfgValue, std::stringstream& ss,
				bool forceDeepByStoredDeepValue = false, int storedDeep = -2);

		CFG_API
		std::string valueToString(unsigned int deep, const Value& cfgValue,
				bool forceDeepByStoredDeepValue = false, int storedDeep = -2);

		CFG_API
		void nameValuePairToStringStream(unsigned int deep,
				const NameValuePair& cfgPair, std::stringstream& ss,
				bool forceDeepByStoredDeepValue = false);

		CFG_API
		std::string nameValuePairToString(unsigned int deep,
				const NameValuePair& cfgPair,
				bool forceDeepByStoredDeepValue = false);
	};
}

#endif

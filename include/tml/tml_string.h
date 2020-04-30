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
				const Value& cfgValue, std::stringstream& ss);

		CFG_API
		std::string valueToString(unsigned int deep, const Value& cfgValue);

		CFG_API
		void nameValuePairToStringStream(unsigned int deep,
				const NameValuePair& cfgPair, std::stringstream& ss);

		CFG_API
		std::string nameValuePairToString(unsigned int deep,
				const NameValuePair& cfgPair);
	};
}

#endif

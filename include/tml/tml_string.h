#ifndef CFG_TML_STRING_H
#define CFG_TML_STRING_H

#include <cfg/export.h>

#include <string>
#include <ostream>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace tmlstring
	{
		CFG_API
		void valueToStream(unsigned int deep,
				const Value& cfgValue, std::ostream& s,
				bool forceDeepByStoredDeepValue = false, int storedDeep = -2);

		CFG_API
		std::string valueToString(unsigned int deep, const Value& cfgValue,
				bool forceDeepByStoredDeepValue = false, int storedDeep = -2);

		/**
		 * @param cfgValue Must be a simple value, a simple array (no complex array), none (empty) or comment.
		 *        An object or complex array is not allowed.
		 * @return Return a tml string (no new line at the end)
		 */
		CFG_API
		std::string plainValueToString(const Value& cfgValue);

		CFG_API
		void nameValuePairToStream(unsigned int deep,
				const NameValuePair& cfgPair, std::ostream& s,
				bool forceDeepByStoredDeepValue = false);

		CFG_API
		std::string nameValuePairToString(unsigned int deep,
				const NameValuePair& cfgPair,
				bool forceDeepByStoredDeepValue = false);
	};
}

#endif

#ifndef CFG_TML_STRING_H
#define CFG_TML_STRING_H

#include <cfg/export.h>

#include <string>
#include <ostream>
#include <vector>
#include <memory>

namespace cfg
{
	class Value;
	class NameValuePair;

	class TmlLines;

	class CFG_API TmlLine
	{
	public:
		int mDeep = 0;
		std::string mLine;
		std::unique_ptr<TmlLines> mSubLines;

		explicit TmlLine(int deep, const std::string& line)
				:mDeep(deep), mLine(line), mSubLines() {}
	};

	class CFG_API TmlLines
	{
	public:
		std::vector<TmlLine> mLines;
	};

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
		 * Convert the cfgValue to TmlLines. The advantage of TmlLines
		 * against output stream or string is that TmlLines is organized as
		 * a graph like cfgValue.
		 *
		 * Using valueToTmlLines and tmlLinesToStream() is the same like
		 * calling directly valueToStream().
		 * Using valueToTmlLines and tmlLinesToString() is the same like
		 * calling directly valueToString().
		 */
		CFG_API
		void valueToTmlLines(unsigned int deep,
				const Value& cfgValue, TmlLines& tmlLines,
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

		CFG_API
		void nameValuePairToTmlLines(unsigned int deep,
				const NameValuePair& cfgPair, TmlLines& tmlLines,
				bool forceDeepByStoredDeepValue = false);

		/**
		 * Add tmlLines to the stream s.
		 */
		CFG_API
		void tmlLinesToStream(const TmlLines& tmlLines, std::ostream& s);
		CFG_API
		std::string tmlLinesToString(const TmlLines& tmlLines);
	};
}

#endif

#ifndef CFG_BTML_STREAM_H
#define CFG_BTML_STREAM_H

#include <cfg/export.h>
#include <vector>
#include <string>
#include <stdint.h>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace btmlstream
	{
		/**
		 * @return Count of added bytes
		 */
		CFG_API
		unsigned int valueToStream(const Value& cfgValue,
				std::vector<uint8_t>& s);

		CFG_API
		unsigned int valueToStreamWithHeader(const Value& cfgValue,
				std::vector<uint8_t>& s, bool useStringTable);

		// return count of used bytes, 0 for error
		CFG_API
		unsigned int streamToValue(const void* stream, unsigned int n,
				Value& cfgValue, std::string* errMsg);

		// return count of used bytes, 0 for error
		CFG_API
		unsigned int streamToValueWithHeader(const void* stream, unsigned int n,
				Value& cfgValue, std::string* errMsg,
				bool& stringTableExist,
				unsigned int& stringTableEntryCount,
				unsigned int& stringTableSize);

		// return count of used bytes, 0 for error
		CFG_API
		unsigned int streamToValueWithOptionalHeader(const void* stream, unsigned int n,
				Value& cfgValue, std::string* errMsg = nullptr);

		// return count of used bytes, 0 for error
		CFG_API
		unsigned int streamToValueWithOptionalHeader(const void* stream, unsigned int n,
				Value& cfgValue, std::string* errMsg,
				bool& headerExist, bool& stringTableExist,
				unsigned int& stringTableEntryCount,
				unsigned int& stringTableSize);
	}
}

#endif

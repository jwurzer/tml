#include <btml/btml_stream.h>
#include <cfg/cfg.h>
#include <map>
//#include <iostream>

namespace cfg
{
	namespace
	{
		union FloatAsBytes {
			float value;
			uint8_t array[4];
		};
		union Int32AsBytes {
			int32_t value;
			uint8_t array[4];
		};
		union Uint32AsBytes {
			uint32_t value;
			uint8_t array[4];
		};
		union Uint16AsBytes {
			uint16_t value;
			uint8_t array[2];
		};

		union FloatAsUint32 {
			float fp;
			uint32_t uintVal;
		};
		union Int32AsUint32 {
			int32_t intVal;
			uint32_t uintVal;
		};

		// return count of added bytes
		// if pushLength is used for a string then length must also include the 0-termination (--> length = str.length() + 1)
		unsigned int pushLength(std::vector<uint8_t>& s, uint32_t length)
		{
			unsigned int bytes = 0;
			if (length < 255) {
				bytes = 1;
				s.push_back(uint8_t(length));
			}
			else {
				bytes = 5;
				s.push_back(255); // 255 --> a four byte count-field is used.
				Uint32AsBytes uintVal;
				uintVal.value = length;
				s.push_back(uintVal.array[0]);
				s.push_back(uintVal.array[1]);
				s.push_back(uintVal.array[2]);
				s.push_back(uintVal.array[3]);
			}
			return bytes;
		}

		// return count of used bytes
		unsigned int getLength(const uint8_t* s, unsigned int n, uint32_t& outLength)
		{
			if (n < 1) {
				return 0;
			}
			uint32_t len = s[0];
			if (len < 255) {
				outLength = len;
				return 1;
			}
			// --> len is 255 --> using 4 bytes for length
			if (n < 5) {
				return 0;
			}
			len = *reinterpret_cast<const uint32_t*>(s + 1);
			outLength = len;
			return 5;
		}

		// return count of used bytes
		unsigned int getLength16(const uint8_t* s, unsigned int n, uint16_t& outLength)
		{
			if (n < 1) {
				return 0;
			}
			uint16_t len = s[0];
			if (len < 255) {
				outLength = len;
				return 1;
			}
			// --> len is 255 --> using 4 bytes for length
			if (n < 3) {
				return 0;
			}
			len = *reinterpret_cast<const uint16_t*>(s + 1);
			outLength = len;
			return 3;
		}

		typedef std::map<std::string, unsigned int /* duplicate count */> TStringCountMap;
		typedef std::map<std::string, unsigned int /* offset */> TStringTableByString;
		typedef std::map<unsigned int /* offset */, std::string> TStringTableByOffset;

		void addToStringTable(const Value& cfgValue, TStringCountMap& stringCountMap)
		{
			switch (cfgValue.mType) {
				case Value::TYPE_TEXT:
				case Value::TYPE_COMMENT:
					++stringCountMap[cfgValue.mText];
					break;
				case Value::TYPE_ARRAY: {
					std::size_t cnt = cfgValue.mArray.size();
					for (std::size_t i = 0; i < cnt; ++i) {
						addToStringTable(cfgValue.mArray[i], stringCountMap);
					}
					break;
				}
				case Value::TYPE_OBJECT: {
					std::size_t cnt = cfgValue.mObject.size();
					for (std::size_t i = 0; i < cnt; ++i) {
						addToStringTable(cfgValue.mObject[i].mName, stringCountMap);
						addToStringTable(cfgValue.mObject[i].mValue, stringCountMap);
					}
					break;
				}
				default:
					break;
			}
		}

		bool createStringTable(const Value& cfgValue,
				TStringTableByString& stringTable,
				std::vector<uint8_t>& s)
		{
			stringTable.clear();
			TStringCountMap stringCountMap;
			addToStringTable(cfgValue, stringCountMap);
			const unsigned int tableBegin = static_cast<unsigned int>(s.size());
			if (tableBegin > 0xffff - 2) {
				return false;
			}
			// add two bytes for count of string entries at string table
			s.push_back(0);
			s.push_back(0);
			unsigned int stringCountForTable = 0;
			unsigned int nextOffset = static_cast<unsigned int>(s.size());
			for (TStringCountMap::const_iterator it = stringCountMap.cbegin();
					it != stringCountMap.cend(); ++it) {
				if (it->second > 1 && it->first.length() >= 2 && it->first.length() <= 32000) {
					if (nextOffset > 0xffff) {
						//std::cout << "Warning: Can't add all duplicated strings to string lookup table" << std::endl;
						break;
					}
					//std::cout << it->first << ": " << it->second << std::endl;
					++stringCountForTable;
					unsigned int length = static_cast<unsigned int>(it->first.length()) + 1; // +1 for null termination
					if (length < 255) {
						s.push_back(uint8_t(length));
					}
					else {
						s.push_back(255); // 255 --> a two byte count-field is used. (at string table only two and not four bytes)
						Uint16AsBytes uintVal;
						uintVal.value = uint16_t(length);
						s.push_back(uintVal.array[0]);
						s.push_back(uintVal.array[1]);
					}
					const unsigned char* str = reinterpret_cast<const unsigned char*>(it->first.c_str());
					for (unsigned int i = 0; i < length; ++i) {
						s.push_back(str[i]);
					}
					stringTable[it->first] = nextOffset;
					nextOffset = static_cast<unsigned int>(s.size());
				}
			}
			if (stringCountForTable > 0xffff) {
				s.resize(tableBegin);
				return false;
			}
			Uint16AsBytes uintVal;
			uintVal.value = uint16_t(stringCountForTable);
			s[tableBegin] = uintVal.array[0];
			s[tableBegin + 1] = uintVal.array[1];
			//std::cout << "different strings: " << stringCountMap.size() <<
			//		", diff strings at table " << stringCountForTable <<
			//		", added bytes " << (s.size() - tableBegin) << std::endl;
			return true;
		}

		// return count of used bytes, 0 for error
		unsigned int loadStringTable(const uint8_t* s, unsigned int n,
				TStringTableByOffset* stringTable, std::string* errMsg,
				unsigned int& stringTableEntryCount)
		{
			stringTableEntryCount = 0;
			if (stringTable) {
				stringTable->clear();
			}
			if (n < 2) {
				if (errMsg) {
					*errMsg += "n < 2 is not allowed\n";
				}
				return 0;
			}
			const uint8_t* const start = s;
			unsigned int strCount = *reinterpret_cast<const uint16_t*>(s);
			s += 2;
			n -= 2;
			//std::cout << "Count is " << strCount << std::endl;
			for (unsigned int i = 0; i < strCount; ++i) {
				if (n < 3) {
					if (errMsg) {
						*errMsg += "failed at string with index " + std::to_string(i) + "\n";
					}
					if (stringTable) {
						stringTable->clear();
					}
					return 0;
				}
				uint16_t len = 0;
				unsigned int bytes = getLength16(s, n, len);
				if (!bytes || !len) {
					if (errMsg) {
						*errMsg += "string with index " + std::to_string(i) + " failed (bytes and/or len is 0)\n";
					}
					if (stringTable) {
						stringTable->clear();
					}
					return 0;
				}
				if (n - bytes < len) { // no -1 because stringTable doesn't store the string type
					if (errMsg) {
						*errMsg += "string with index " + std::to_string(i) + " failed (string too long)\n";
					}
					return 0;
				}
				const char* str = reinterpret_cast<const char*>(s + bytes);
				if (str[len - 1] != '\0') {
					if (errMsg) {
						*errMsg += "string with index " + std::to_string(i) + " failed (string not null)\n";
					}
					return 0;
				}
				unsigned int offset = static_cast<unsigned int>(s - start);
				if (stringTable) {
					(*stringTable)[offset] = str;
				}
				s += bytes + len;
				n -= bytes + len;
				++stringTableEntryCount;
			}
			return static_cast<unsigned int>(s - start);
		}

		// return count of used bytes, 0 for error
		unsigned int bytesToValue(const uint8_t* s, unsigned int n,
				Value& cfgValue, const uint8_t* stringTable)
		{
			if (!n) {
				return 0;
			}
			Value::EValueType valueType = static_cast<Value::EValueType>(s[0] & 0x0f);
			switch (valueType) {
				case Value::TYPE_NONE:
					cfgValue.clear();
					return 1;
				case Value::TYPE_NULL:
					cfgValue.setNull();
					return 1;
				case Value::TYPE_BOOL:
					if (n < 2) {
						return 0;
					}
					cfgValue.setBool(s[1] > 0);
					return 2;
				case Value::TYPE_FLOAT: {
					if (n < 5) {
						return 0;
					}
					FloatAsUint32 f;
					f.uintVal = *reinterpret_cast<const uint32_t*>(s + 1);
					cfgValue.setFloatingPoint(f.fp);
					return 5;
				}
				case Value::TYPE_INT: {
					if (n < 5) {
						return 0;
					}
					Int32AsUint32 f;
					f.uintVal = *reinterpret_cast<const uint32_t*>(s + 1);
					cfgValue.setInteger(f.intVal);
					return 5;
				}
				case Value::TYPE_TEXT:
				case Value::TYPE_COMMENT: {
					if (n < 3) {
						return 0;
					}
					if (stringTable && s[1] == 0) {
						// --> string reference is used
						if (n < 4) {
							return 0;
						}
						uint16_t offset = *reinterpret_cast<const uint16_t*>(s + 2);
						if (!offset) {
							// offset of 0 is not allowed because
							// the string table as 6 bytes for header and
							// 2 bytes for string count.
							return 0;
						}
						const uint8_t* refStr = stringTable + offset;
						uint16_t len = 0;
						unsigned int bytes = getLength16(refStr, 3, len);
						const char* str = reinterpret_cast<const char*>(refStr + bytes);
						if (str[len - 1] != '\0') {
							return 0;
						}
						if (valueType == Value::TYPE_TEXT) {
							bool parseTextWithQuotes = ((s[0] & 0x10) != 0);
							cfgValue.setText(str, parseTextWithQuotes);
						}
						else {
							cfgValue.setComment(str);
						}
						return 4; // 1 (type) + 1 (ref marker 0) + 2 (offset)
					}
					else {
						// --> normal string
						uint32_t len = 0;
						unsigned int bytes = getLength(s + 1, n - 1, len);
						if (!bytes || !len) {
							// length of 0 is not allowed because an empty string
							// has length 1 which includes the 0-termination byte.
							// A empty string without 0-termination (length 0) is not
							// allowed.
							return 0;
						}
						if (n - 1 - bytes < len) {
							return 0;
						}
						const char* str = reinterpret_cast<const char*>(s + 1 + bytes);
						if (str[len - 1] != '\0') {
							return 0;
						}
						if (valueType == Value::TYPE_TEXT) {
							bool parseTextWithQuotes = ((s[0] & 0x10) != 0);
							cfgValue.setText(str, parseTextWithQuotes);
						}
						else {
							cfgValue.setComment(str);
						}
						return 1 + bytes + len;
					}
				}
				case Value::TYPE_ARRAY: {
					if (n < 2) {
						return 0;
					}
					uint32_t count = 0;
					unsigned int bytes = getLength(s + 1, n - 1, count);
					if (!bytes) {
						return 0;
					}
					++bytes; // for TYPE_ARRAY byte
					s += bytes;
					n -= bytes;
					cfgValue.setArray();
					cfgValue.mArray.resize(count);
					for (uint32_t i = 0; i < count; ++i) {
						unsigned int nextBytes = bytesToValue(s, n, cfgValue.mArray[i], stringTable);
						if (!nextBytes) {
							return 0;
						}
						bytes += nextBytes;
						s += nextBytes;
						n -= nextBytes;
					}
					return bytes;
				}
				case Value::TYPE_OBJECT: {
					if (n < 2) {
						return 0;
					}
					uint32_t count = 0;
					unsigned int bytes = getLength(s + 1, n - 1, count);
					if (!bytes) {
						return 0;
					}
					++bytes; // for TYPE_OBJECT byte
					s += bytes;
					n -= bytes;
					cfgValue.setObject();
					cfgValue.mObject.resize(count);
					for (uint32_t i = 0; i < count; ++i) {
						unsigned int nextBytes = bytesToValue(s, n, cfgValue.mObject[i].mName, stringTable);
						if (!nextBytes) {
							return 0;
						}
						bytes += nextBytes;
						s += nextBytes;
						n -= nextBytes;
						nextBytes = bytesToValue(s, n, cfgValue.mObject[i].mValue, stringTable);
						if (!nextBytes) {
							return 0;
						}
						bytes += nextBytes;
						s += nextBytes;
						n -= nextBytes;
					}
					return bytes;
				}
			}
			return 0; // should not be possible
		}

		unsigned int valueToStreamOptStringTable(const Value& cfgValue,
				std::vector<uint8_t>& s, const TStringTableByString* stringTable)
		{
			switch (cfgValue.mType) {
				case Value::TYPE_NONE:
					s.push_back(uint8_t(Value::TYPE_NONE));
					return 1;
				case Value::TYPE_NULL:
					s.push_back(uint8_t(Value::TYPE_NULL));
					return 1;
				case Value::TYPE_BOOL:
					s.push_back(uint8_t(Value::TYPE_BOOL));
					s.push_back(uint8_t(cfgValue.mBool));
					return 2;
				case Value::TYPE_FLOAT: {
					s.push_back(uint8_t(Value::TYPE_FLOAT));
					FloatAsBytes fp;
					fp.value = cfgValue.mFloatingPoint;
					s.push_back(fp.array[0]);
					s.push_back(fp.array[1]);
					s.push_back(fp.array[2]);
					s.push_back(fp.array[3]);
					return 5;
				}
				case Value::TYPE_INT: {
					s.push_back(uint8_t(Value::TYPE_INT));
					Int32AsBytes intVal;
					intVal.value = cfgValue.mInteger;
					s.push_back(intVal.array[0]);
					s.push_back(intVal.array[1]);
					s.push_back(intVal.array[2]);
					s.push_back(intVal.array[3]);
					return 5;
				}
				case Value::TYPE_TEXT:
				case Value::TYPE_COMMENT: {
					unsigned int bytes = 1;
					uint8_t typeByte = uint8_t(cfgValue.mType);
					if (cfgValue.mType == Value::TYPE_TEXT) {
						if (cfgValue.mParseTextWithQuotes) {
							typeByte |= 0x10; // set flag for "parse text with quotes"
						}
					}
					s.push_back(typeByte);
					if (stringTable) {
						TStringTableByString::const_iterator it = stringTable->find(cfgValue.mText);
						if (it != stringTable->cend()) {
							// --> found string at string table
							// --> use string from string table
							bytes += 3;
							s.push_back(0); // --> ref to string table entry
							Uint16AsBytes uintVal;
							uintVal.value = uint16_t(it->second); // offset/ref to string
							s.push_back(uintVal.array[0]);
							s.push_back(uintVal.array[1]);
							return bytes;
						}
					}
					std::size_t len = cfgValue.mText.length();
					++len; // +1 for the \0 termination
					const unsigned char* str = reinterpret_cast<const unsigned char*>(cfgValue.mText.c_str());
					bytes += pushLength(s, uint32_t(len));
					for (std::size_t i = 0; i < len; ++i) {
						s.push_back(str[i]);
					}
					return bytes + static_cast<unsigned int>(len);
				}
				case Value::TYPE_ARRAY: {
					unsigned int bytes = 1;
					s.push_back(uint8_t(Value::TYPE_ARRAY));
					std::size_t cnt = cfgValue.mArray.size();
					bytes += pushLength(s, uint32_t(cnt));
					for (std::size_t i = 0; i < cnt; ++i) {
						bytes += valueToStreamOptStringTable(cfgValue.mArray[i], s, stringTable);
					}
					return bytes;
				}
				case Value::TYPE_OBJECT: {
					unsigned int bytes = 1;
					s.push_back(uint8_t(Value::TYPE_OBJECT));
					std::size_t cnt = cfgValue.mObject.size();
					bytes += pushLength(s, uint32_t(cnt));
					for (std::size_t i = 0; i < cnt; ++i) {
						bytes += valueToStreamOptStringTable(cfgValue.mObject[i].mName, s, stringTable);
						bytes += valueToStreamOptStringTable(cfgValue.mObject[i].mValue, s, stringTable);
					}
					return bytes;
				}
			}
			return 0;
		}
	}
}

unsigned int cfg::btmlstream::valueToStream(
		const Value& cfgValue, std::vector<uint8_t>& s)
{
	return valueToStreamOptStringTable(cfgValue, s, nullptr);
}

unsigned int cfg::btmlstream::valueToStreamWithHeader(const Value& cfgValue,
		std::vector<uint8_t>& s, bool useStringTable)
{
	s.clear();
	s.push_back('b');
	s.push_back('t');
	s.push_back('m');
	s.push_back('l');
	s.push_back(1); // version
	s.push_back(useStringTable ? 1 : 0);
	TStringTableByString stringTable;
	if (useStringTable) {
		if (!createStringTable(cfgValue, stringTable, s)) {
			s.clear();
			return 0;
		}
	}
	unsigned int bytes = static_cast<unsigned int>(s.size());
	return valueToStreamOptStringTable(cfgValue, s, useStringTable ? &stringTable : nullptr) + bytes;
}

unsigned int cfg::btmlstream::streamToValue(const void* stream, unsigned int n,
		Value& cfgValue, std::string* errMsg)
{
	cfgValue.clear();
	if (!stream || !n) {
		return 0;
	}
	const uint8_t* s = static_cast<const uint8_t*>(stream);
	unsigned int rv = bytesToValue(s, n, cfgValue, nullptr);
	if (!rv && errMsg) {
		*errMsg += "bytesToValue() failed\n";
	}
	return rv;
}

unsigned int cfg::btmlstream::streamToValueWithHeader(const void* stream,
		unsigned int n, Value& cfgValue, std::string* errMsg,
		bool& stringTableExist, unsigned int& stringTableEntryCount,
		unsigned int& stringTableSize)
{
	stringTableExist = false;
	stringTableEntryCount = 0;
	stringTableSize = 0;
	cfgValue.clear();
	if (!stream || n < 6) {
		return 0;
	}
	const uint8_t* s = static_cast<const uint8_t*>(stream);
	if (s[0] != 'b' || s[1] != 't' || s[2] != 'm' || s[3] != 'l') {
		return 0;
	}
	if (s[4] != 1) {
		// only version 1 is supported
		return 0;
	}
	if (s[5] != 0 && s[5] != 1) {
		// only 0 or 1 is allowed
		return 0;
	}
	bool useStringTable = (s[5] == 1);
	const uint8_t* stringTable = nullptr;
	unsigned int tableSize = 0;
	if (useStringTable) {
		if (n < 8) {
			return 0;
		}
		stringTable = s;
		unsigned int entryCount = 0;
		tableSize = loadStringTable(s + 6, n - 6, nullptr, errMsg, entryCount);
		if (!tableSize) {
			if (errMsg) {
				*errMsg += "Can't load string table.\n";
			}
			return 0;
		}
		stringTableExist = true;
		stringTableEntryCount = entryCount;
		stringTableSize = tableSize;
	}
	unsigned int rv = bytesToValue(s + 6 + tableSize, n - 6 - tableSize, cfgValue, stringTable);
	if (!rv) {
		return 0;
	}
	return rv + 6 + tableSize;
}

unsigned int cfg::btmlstream::streamToValueWithOptionalHeader(
		const void* stream, unsigned int n, Value& cfgValue,
		std::string* errMsg)
{
	bool headerExist = false;
	bool stringTableExist = false;
	unsigned int stringTableEntryCount = 0;
	unsigned int stringTableSize = 0;

	return btmlstream::streamToValueWithOptionalHeader(stream, n, cfgValue,
			errMsg, headerExist, stringTableExist, stringTableEntryCount,
			stringTableSize);
}

unsigned int cfg::btmlstream::streamToValueWithOptionalHeader(
		const void* stream, unsigned int n, Value& cfgValue,
		std::string* errMsg, bool& headerExist, bool& stringTableExist,
		unsigned int& stringTableEntryCount, unsigned int& stringTableSize)
{
	headerExist = false;
	stringTableExist = false;
	stringTableEntryCount = 0;
	stringTableSize = 0;

	const uint8_t* s = static_cast<const uint8_t*>(stream);
	bool useHeaderVersion = (n >= 6 && s[0] == 'b' && s[1] == 't' && s[2] == 'm' && s[3] == 'l');
	if (useHeaderVersion) {
		headerExist = true;
		return streamToValueWithHeader(stream, n, cfgValue, errMsg,
				stringTableExist, stringTableEntryCount, stringTableSize);
	}
	else {
		return streamToValue(stream, n, cfgValue, errMsg);
	}
}

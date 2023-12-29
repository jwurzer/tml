#ifndef CFG_BTML_STREAM_H
#define CFG_BTML_STREAM_H

#include <cfg/export.h>
#include <vector>
#include <string>
#include <stdint.h>

/*
 btml - binary tml

 format:
 <header>                (optional)
 <string lookup table>   (optional)
 <type> <data>           (multiple times)

 header: 6 bytes
  format:
  0: 'b'
  1: 't'
  2: 'm'
  3: 'l'
  4: 0x1          - version
  5: 0x0 or 0x1   - if string lookup table is stored/used. 1 --> stored/used, 0 --> not stored/not used.

 string lookup table:
  At the string lookup table only strings are added which are used
  multiple times (at least two times) and has a length >= 2 and <= 32000.
  format:
  <string entry count>        - 2 bytes for string entry count (little endian).
  <string length>             - 1 byte or 3 bytes. Length includes the 0-termination!!!
                                If length is < 255 then 1 byte is used.
                                If length is >= 255 then 3 bytes are used.
                                First byte is 255. Second and third are used for length (little endian).
  <string data/text>          - <length> bytes. Data of the text inclusive the 0-termination.
  <string length>             - Length for next string.
  <string data/text>          - Data/Text from next string.

  The count of <string length> and <string data/text> combinations is <string entry count>

 <type> <data>:           (multiple times)
 Is used to store the different cfg::Value's.
 format:
  <type>            - 1 byte. Bits 7(MSB),6,5,4 are used for flags.
                      Bits 3,2,1,0(LSB) are used to store the type (enum EValueType).
  <data>            - Depends on stored type.

  Value::TYPE_NONE:  - Only type byte. All flags are 0. No data. --> Total 1 byte
  Value::TYPE_NULL:  - Only type byte. All flags are 0. No data. --> Total 1 byte
  Value::TYPE_BOOL:  - Type byte. All flags are 0. 1 byte for data (0 = false, 1 = true). --> Total 2 byte
  Value::TYPE_FLOAT: - Type byte. All flags are 0. 4 byte for float data. --> Total 5 byte
  Value::TYPE_INT:   - Type byte. All flags are 0. 4 byte for int data (little endian). --> Total 5 byte
  Value::TYPE_TEXT / Value::TYPE_COMMENT:
    Type byte: Bits 3 - 0 stores TYPE_TEXT or TYPE_COMMENT.
               Bits 7 - 4 stores the flags. All 0 for TYPE_COMMENT.
               Flag 4: stores if TYPE_TEXT is "parse text with quotes"
    Data:
    Length/Ref: 1 or 5 bytes for length (includes 0 termination) or 3 bytes for ref.
               If first byte is > 0 and < 255 then 1 byte for length is used.
               If first byte is 255 then 4 bytes are used for length (little endian).
               If first byte is 0 then it is a string reference.
               The next 2 bytes stores the offset from btml buffer beginning to the string at the string lookup table.
    String-data: Data of string incl 0 termination. No data if it is a string reference.
               In this case the data are loaded from the string lookup table.
  Value::TYPE_ARRAY:
    Type byte: All flags are 0. Stores type TYPE_ARRAY.
    Data: 1 or 5 bytes for array element count (element count and NOT size in bytes!)
          btml encoded values for array elements.
  Value::TYPE_OBJECT:
    Type byte: All flags are 0. Stores type TYPE_ARRAY.
    Data: 1 or 5 bytes for object's name-value pair count (pair count and NOT size in bytes!)
          btml encoded value for name.
          btml encoded value for value.
          name-value-pair repairs for all name-value pairs of the object.
*/

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

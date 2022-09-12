#include <cfg/cfg_enum_string.h>

const char* cfg::enumstring::getValueTypeAsString(Value::EValueType valueType)
{
	const char* strType[] = {
			"TYPE_NONE",
			"TYPE_NULL",
			"TYPE_BOOL",
			"TYPE_FLOAT",
			"TYPE_INT",
			"TYPE_TEXT",
			"TYPE_COMMENT",
			"TYPE_ARRAY",
			"TYPE_OBJECT",

			"error"
	};
	return strType[(valueType < 9) ? valueType : 9];
}

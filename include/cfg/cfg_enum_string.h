#ifndef CFG_CFG_ENUM_STRING_H
#define CFG_CFG_ENUM_STRING_H

#include <cfg/export.h>
#include <cfg/cfg.h>

namespace cfg
{
	namespace enumstring
	{
		CFG_API
		const char* getValueTypeAsString(Value::EValueType valueType);
	}
}

#endif

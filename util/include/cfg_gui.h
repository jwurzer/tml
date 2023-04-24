#ifndef UTIL_CFG_GUI_H
#define UTIL_CFG_GUI_H

#include <cfg/export.h>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace gui
	{
		CFG_API
		void valueAsImguiTree(const Value &cfgValue);
		CFG_API
		void nameValuePairAsImguiTree(const NameValuePair &cfgPair);

		CFG_API
		void valueAsImguiText(const Value &cfgValue);
		CFG_API
		void nameValuePairAsImguiText(const NameValuePair &cfgPair);
	}
}

#endif

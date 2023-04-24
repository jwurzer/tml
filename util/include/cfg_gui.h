#ifndef UTIL_CFG_GUI_H
#define UTIL_CFG_GUI_H

#include <cfg/export.h>
#include <cfg/cfg_template.h>
#include <cfg/cfg_translation.h>
#include <string>
#include <stdarg.h>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace gui
	{
		CFG_API
		void intentTextV(const char* fmt, va_list args);
		CFG_API
		void intentText(const char* fmt, ...);

		CFG_API
		void valueAsImguiTree(const std::string& internalLabelId,
				const Value &cfgValue, bool allowMultipleEmptyLines = true);
		CFG_API
		void nameValuePairAsImguiTree(const std::string& internalLabelId,
				const NameValuePair &cfgPair,
				bool allowMultipleEmptyLines = true);

		CFG_API
		void valueAsImguiText(const Value &cfgValue);
		CFG_API
		void nameValuePairAsImguiText(const NameValuePair &cfgPair);

		CFG_API
		void templateMapAsImguiTree(const std::string& internalLabelId,
				const cfg::cfgtemp::TemplateMap& templateMap);

		CFG_API
		void languageMapAsImguiTree(const std::string& internalLabelId,
				const cfg::cfgtr::LanguageMap& languageMap);
	}
}

#endif

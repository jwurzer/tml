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

	class GuiRenderOptions
	{
	public:
		bool mShowAdditionalTemplateInfosAtObjectName = true;
		// -1 ... no limit (all), 0 ... zero (none), >= 1 ... count of limit
		int mMultipleEmptyLineLimit = -1;
		// e.g. flag ImGuiTreeNodeFlags_DefaultOpen can be set if all tree nodes
		// should be open at the beginning for valueAsImguiTree() or
		// nameValuePairAsImguiTree().
		int mImGuiTreeNodeFlags = 0;
		// is only used if mShowAdditionalTemplateInfosAtObjectName is true
		// otherwise it is ignored and mImGuiTreeNodeFlags is also used for templates.
		int mImGuiTreeNodeFlagsForTemplate = 0;
	};

	namespace gui
	{
		CFG_API
		void intentTextV(const char* fmt, va_list args);
		CFG_API
		void intentText(const char* fmt, ...);

		CFG_API
		void valueAsImguiTree(const std::string& internalLabelId,
				const Value &cfgValue,
				const GuiRenderOptions& options = GuiRenderOptions{});
		CFG_API
		void nameValuePairAsImguiTree(const std::string& internalLabelId,
				const NameValuePair &cfgPair,
				const GuiRenderOptions& options = GuiRenderOptions{});

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

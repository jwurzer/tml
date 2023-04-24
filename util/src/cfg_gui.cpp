#include <cfg_gui.h>
#include <cfg/cfg.h>
#include <tml/tml_string.h>
#include <imgui.h>
#include <sstream> // TODO should not be necessary because of tml/tml_string.h --> include in tml_string.h

namespace cfg
{
	namespace gui
	{
		namespace
		{
			void IntentTextV(const char* fmt, va_list args)
			{
				ImGui::Indent();
				ImGui::TextV(fmt, args);
				ImGui::Unindent();
			}

			void IntentText(const char* fmt, ...)
			{
				va_list args;
				va_start(args, fmt);
				IntentTextV(fmt, args);
				va_end(args);
			}

			void insertNameValuePair(const NameValuePair &cfgPair, size_t index);

			void insertValue(const Value &cfgValue, bool useNameAsLabel,
					const std::string& name, size_t index)
			{
				bool isComplexArr = cfgValue.isComplexArray();
				if ((cfgValue.isObject() && !cfgValue.mObject.empty()) || isComplexArr) {
					if (useNameAsLabel) {
						char strId[32];
						snprintf(strId, 32, "node_%zu", index);
						bool isOpened = ImGui::TreeNode(strId, isComplexArr ? "%s = []" : "%s", name.c_str());
						if (!isOpened) {
							return;
						}
					}
					if (cfgValue.isObject()) {
						size_t cnt = cfgValue.mObject.size();
						for (size_t i = 0; i < cnt; ++i) {
							insertNameValuePair(cfgValue.mObject[i], i);
						}
					}
					else {
						size_t cnt = cfgValue.mArray.size();
						for (size_t i = 0; i < cnt; ++i) {
							std::string name = "(element " + std::to_string(i) + ")";
							insertValue(cfgValue.mArray[i], true, name, i);
						}
					}
					if (useNameAsLabel) {
						ImGui::TreePop();
					}
					return;
				}

				IntentText("%s", tmlstring::plainValueToString(cfgValue).c_str());
			}

			void insertNameValuePair(const NameValuePair &cfgPair, size_t index)
			{
				if (cfgPair.mName.isEmpty() && cfgPair.mValue.isEmpty()) {
					IntentText("");
					return;
				}

				if (cfgPair.mName.isComment() && cfgPair.mValue.isEmpty()) {
					IntentText("#%s", cfgPair.mName.mText.c_str());
					return;
				}

				std::string name;
				if (cfgPair.mName.isObject()) {
					name = "object (not allowed)";
				}
				else if (cfgPair.mName.isComplexArray()) {
					name = "complex array (not allowed)";
				}
				else {
					name = tmlstring::plainValueToString(cfgPair.mName);
				}

				if ((cfgPair.mValue.isObject() && !cfgPair.mValue.mObject.empty()) ||
						cfgPair.mValue.isComplexArray()) {
					insertValue(cfgPair.mValue, true, name, index);
					return;
				}

				if (cfgPair.mValue.isEmpty()) {
					IntentText("%s", name.c_str());
				}
				else {
					IntentText("%s = %s", name.c_str(),
							tmlstring::plainValueToString(cfgPair.mValue).c_str());
				}
			}
		}
	}
}

void cfg::gui::valueAsImguiTree(const Value &cfgValue)
{
	insertValue(cfgValue, false, "", 0);
}

void cfg::gui::nameValuePairAsImguiTree(const NameValuePair &cfgPair)
{
	insertNameValuePair(cfgPair, 0);
}

void cfg::gui::valueAsImguiText(const Value &cfgValue)
{
	std::stringstream ss;
	tmlstring::valueToStream(0, cfgValue, ss);
	std::string line;
    while (std::getline(ss, line)) {
        ImGui::Text("%s", line.c_str());
    }
}

void cfg::gui::nameValuePairAsImguiText(const NameValuePair &cfgPair)
{
	std::stringstream ss;
	tmlstring::nameValuePairToStream(0, cfgPair, ss);
	std::string line;
	while (std::getline(ss, line)) {
		ImGui::Text("%s", line.c_str());
	}
}

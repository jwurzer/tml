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
			void insertNameValuePair(const NameValuePair &cfgPair, size_t index,
					bool allowMultipleEmptyLines,
					unsigned int& curContinuousEmptyCount);

			void insertValue(const Value &cfgValue, bool useNameAsLabel,
					const std::string& name, size_t index,
					bool allowMultipleEmptyLines,
					unsigned int& curContinuousEmptyCount)
			{
				bool isComplexArr = cfgValue.isComplexArray();
				if ((cfgValue.isObject() && !cfgValue.mObject.empty()) || isComplexArr) {
					if (useNameAsLabel) {
						curContinuousEmptyCount = 0;
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
							insertNameValuePair(cfgValue.mObject[i], i,
									allowMultipleEmptyLines,
									curContinuousEmptyCount);
						}
					}
					else {
						size_t cnt = cfgValue.mArray.size();
						for (size_t i = 0; i < cnt; ++i) {
							std::string name = "(element " + std::to_string(i) + ")";
							insertValue(cfgValue.mArray[i], true, name, i,
									allowMultipleEmptyLines,
								   curContinuousEmptyCount);
						}
					}
					if (useNameAsLabel) {
						ImGui::TreePop();
					}
					return;
				}

				std::string plainValue = tmlstring::plainValueToString(cfgValue);
				if (plainValue.empty()) {
					++curContinuousEmptyCount;
					if (allowMultipleEmptyLines || curContinuousEmptyCount <= 1) {
						intentText("%s", plainValue.c_str());
					}
				}
				else {
					curContinuousEmptyCount = 0;
					intentText("%s", plainValue.c_str());
				}
			}

			void insertNameValuePair(const NameValuePair &cfgPair, size_t index,
					bool allowMultipleEmptyLines,
					unsigned int& curContinuousEmptyCount)
			{
				if (cfgPair.mName.isEmpty() && cfgPair.mValue.isEmpty()) {
					++curContinuousEmptyCount;
					if (allowMultipleEmptyLines || curContinuousEmptyCount <= 1) {
						intentText("");
					}
					return;
				}

				if (cfgPair.mName.isComment() && cfgPair.mValue.isEmpty()) {
					curContinuousEmptyCount = 0;
					intentText("#%s", cfgPair.mName.mText.c_str());
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
					insertValue(cfgPair.mValue, true, name, index,
							allowMultipleEmptyLines, curContinuousEmptyCount);
					return;
				}

				curContinuousEmptyCount = 0;
				if (cfgPair.mValue.isEmpty()) {
					intentText("%s", name.c_str());
				}
				else {
					intentText("%s = %s", name.c_str(),
							tmlstring::plainValueToString(cfgPair.mValue).c_str());
				}
			}
		}
	}
}

void cfg::gui::intentTextV(const char* fmt, va_list args)
{
	ImGui::Indent();
	ImGui::TextV(fmt, args);
	ImGui::Unindent();
}

void cfg::gui::intentText(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	intentTextV(fmt, args);
	va_end(args);
}

void cfg::gui::valueAsImguiTree(const std::string& internalLabelId,
		const Value &cfgValue, bool allowMultipleEmptyLines)
{
	if (!internalLabelId.empty()) {
		ImGui::PushID(internalLabelId.c_str());
	}
	unsigned int curContinuousEmptyCount = 0;
	insertValue(cfgValue, false, "", 0,
			allowMultipleEmptyLines, curContinuousEmptyCount);
	if (!internalLabelId.empty()) {
		ImGui::PopID();
	}
}

void cfg::gui::nameValuePairAsImguiTree(const std::string& internalLabelId,
		const NameValuePair &cfgPair, bool allowMultipleEmptyLines)
{
	if (!internalLabelId.empty()) {
		ImGui::PushID(internalLabelId.c_str());
	}
	unsigned int curContinuousEmptyCount = 0;
	insertNameValuePair(cfgPair, 0,
			allowMultipleEmptyLines, curContinuousEmptyCount);
	if (!internalLabelId.empty()) {
		ImGui::PopID();
	}
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

void cfg::gui::templateMapAsImguiTree(const std::string& internalLabelId,
		const cfg::cfgtemp::TemplateMap& templateMap)
{
	if (!internalLabelId.empty()) {
		ImGui::PushID(internalLabelId.c_str());
	}
	for (const auto& entry : templateMap) {
		std::string strId = "temp-" + entry.first;
		if (ImGui::TreeNode(strId.c_str(), "%s", entry.first.c_str())) {
			const CfgTemplate& cfgTemplate = entry.second;
			intentText("parameter count: %zu", cfgTemplate.getParameterCount());
			std::stringstream ss;
			for (const std::string& param : cfgTemplate.getParameters()) {
				ss << " " << param;
			}
			intentText("parameters: %s", ss.str().c_str());
			const std::vector<NameValuePair>& pairs = cfgTemplate.getPairs();
			std::size_t pairCnt = pairs.size();
			intentText("template pair count: %zu", pairCnt);
			for (std::size_t i = 0; i < pairCnt; ++i) {
				std::string tempStrId = "temp-" + std::to_string(i);
				nameValuePairAsImguiTree(tempStrId, pairs[i]);
			}
			ImGui::TreePop();
		}
	}
	if (!internalLabelId.empty()) {
		ImGui::PopID();
	}
}

void cfg::gui::languageMapAsImguiTree(const std::string& internalLabelId,
		const cfg::cfgtr::LanguageMap& languageMap)
{
	if (!internalLabelId.empty()) {
		ImGui::PushID(internalLabelId.c_str());
	}
	for (const auto& entry : languageMap) {
		std::string strId = "lang-" + entry.first;
		if (ImGui::TreeNode(strId.c_str(), "%s", entry.first.empty() ? "\"\"" : entry.first.c_str())) {
			const cfg::cfgtr::TranslationMap& translationMap = entry.second;
			for (const auto& transEntry : translationMap) {
				std::string transStrId = "trans-" + transEntry.first;
				if (ImGui::TreeNode(transStrId.c_str(), "'%s'", transEntry.first.c_str())) {
					//intentText("id: %s", transEntry.second.mTranslationId.c_str());
					valueAsImguiTree("", transEntry.second.mValue);
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}
	if (!internalLabelId.empty()) {
		ImGui::PopID();
	}
}

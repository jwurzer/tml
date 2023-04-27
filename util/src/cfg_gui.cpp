#include <cfg_gui.h>
#include <cfg/cfg.h>
#include <tml/tml_string.h>
#include <imgui.h>
#include <sstream>
#include <string.h>

namespace cfg
{
	namespace gui
	{
		namespace
		{
			/**
			 * @param out Can be null. If null ImGui::Text() is used otherwise ostream out.
			 */
			void optionalText(const GuiRenderOptions& options, bool forceOutput,
					unsigned int deep, std::ostream* out, const char* fmt, ...)
			{
				if (!options.mUseSearchMode || forceOutput) {
					va_list args;
					va_start(args, fmt);
					if (out) {
						for (unsigned int i = 0; i < deep; ++i) {
							(*out) << "\t";
						}
						char str[128];
						vsnprintf(str, 128, fmt, args);
						str[127] = 0;
						(*out) << str << "\n";
					}
					else {
						intentTextV(fmt, args);
					}
					va_end(args);
				}
			}

			// return true if it was a valid template
			bool changeNameIfTemplateObject(const NameValuePair& nvp,
					std::string& name) {
				bool isPossible = nvp.isObject() &&
						nvp.mName.isText() && nvp.mName.mText == "template" &&
						nvp.mValue.mObject.size() >= 2;
				if (!isPossible) {
					return false;
				}
				std::size_t cnt = nvp.mValue.mObject.size();
				std::size_t nameIndex = cnt;
				std::size_t paramIndex = cnt;
				for (std::size_t i = 0; i < cnt; ++i) {
					const std::string& text = nvp.mValue.mObject[i].mName.mText;
					if (text == "name") {
						if (nameIndex < cnt) {
							// already used. multiple times before parameters not allowed
							return false;
						}
						nameIndex = i;
					}
					else if (text == "parameters") {
						paramIndex = i;
						break;
					}
				}
				if (nameIndex >= cnt || paramIndex >= cnt || nameIndex >= paramIndex) {
					return false;
				}
				char label[128];
				snprintf(label, 128, "%s: %-30s %s",
						nvp.mName.mText.c_str(),
						nvp.mValue.mObject[nameIndex].mValue.mText.c_str(),
						tmlstring::plainValueToString(nvp.mValue.mObject[paramIndex].mValue).c_str());
				label[127] = 0;
				name = label;
				return true;
			}

			/**
			 * @param out Can be null. If null ImGui::Text() is used otherwise ostream out.
			 */
			void insertNameValuePair(const NameValuePair &cfgPair,
					size_t index, std::vector<size_t>* indexStack,
					const GuiRenderOptions& options, unsigned int deep,
					std::ostream* out, unsigned int& curContinuousEmptyCount);

			/**
			 * @param out Can be null. If null ImGui::Text() is used otherwise ostream out.
			 */
			void insertValue(const Value &cfgValue, bool useNameAsLabel,
					const std::string& name, size_t index, std::vector<size_t>* indexStack,
					const GuiRenderOptions& options, unsigned int deep, std::ostream* out,
					unsigned int& curContinuousEmptyCount, bool isTemplate = false)
			{
				bool isComplexArr = cfgValue.isComplexArray();
				if ((cfgValue.isObject() && !cfgValue.mObject.empty()) || isComplexArr) {
					if (useNameAsLabel && options.mUseSearchMode) {
						useNameAsLabel = cfgValue.attributeExist(options.mSearchName, false, true, true);
					}
					if (useNameAsLabel) {
						curContinuousEmptyCount = 0;
						if (out) {
							optionalText(options, true /* because its a node */,
									deep, out, isComplexArr ? "%s = []" : "%s", name.c_str());
						}
						else {
							char strId[64];
							if (options.mUseSearchMode && indexStack) {
								// for search mode tree nodes in the same sub level
								// can have the same index because if some nodes
								// are ignored at the presentation then tree nodes
								// which are normally at different sub trees are
								// can be now in the same sub tree with the same index
								// and will produce the same tree node id.
								// To avoid the same tree node id all parent indexes
								// from the index stack are included.
								std::stringstream ss;
								ss << "node_";
								for (size_t i : *indexStack) {
									ss << "_" << i;
								}
								ss << "_" << index;
								std::string id = ss.str();
								strncpy(strId, id.c_str(), 64);
							}
							else {
								snprintf(strId, 64, "node_%zu", index);
							}
							strId[63] = 0;
							bool isOpened = ImGui::TreeNodeEx(strId,
									isTemplate ? options.mImGuiTreeNodeFlagsForTemplate : options.mImGuiTreeNodeFlags,
									isComplexArr ? "%s = []" : "%s", name.c_str());
							if (!isOpened) {
								return;
							}
						}
						++deep;
					}
					if (cfgValue.isObject()) {
						if (indexStack) {
							indexStack->push_back(index);
						}
						size_t cnt = cfgValue.mObject.size();
						for (size_t i = 0; i < cnt; ++i) {
							insertNameValuePair(cfgValue.mObject[i], i, indexStack,
									options, deep, out, curContinuousEmptyCount);
						}
						if (indexStack) {
							indexStack->pop_back();
						}
					}
					else {
						if (indexStack) {
							indexStack->push_back(index);
						}
						size_t cnt = cfgValue.mArray.size();
						for (size_t i = 0; i < cnt; ++i) {
							std::string name = "(element " + std::to_string(i) + ")";
							insertValue(cfgValue.mArray[i], true, name, i, indexStack,
									options, deep, out, curContinuousEmptyCount);
						}
						if (indexStack) {
							indexStack->pop_back();
						}
					}
					if (useNameAsLabel && !out) {
						ImGui::TreePop();
					}
					return;
				}

				std::string plainValue = tmlstring::plainValueToString(cfgValue);
				bool forceOutput = (options.mUseSearchMode && options.mSearchName == plainValue);
				if (plainValue.empty()) {
					++curContinuousEmptyCount;
					if (options.mMultipleEmptyLineLimit == -1 ||
							int(curContinuousEmptyCount) <= options.mMultipleEmptyLineLimit) {
						optionalText(options, forceOutput, deep, out, "%s", plainValue.c_str());
					}
				}
				else {
					curContinuousEmptyCount = 0;
					optionalText(options, forceOutput, deep, out, "%s", plainValue.c_str());
				}
			}

			/**
			 * @param out Can be null. If null ImGui::Text() is used otherwise ostream out.
			 */
			void insertNameValuePair(const NameValuePair &cfgPair,
					size_t index, std::vector<size_t>* indexStack,
					const GuiRenderOptions& options, unsigned int deep, std::ostream* out,
					unsigned int& curContinuousEmptyCount)
			{
				if (cfgPair.mName.isEmpty() && cfgPair.mValue.isEmpty()) {
					++curContinuousEmptyCount;
					if (options.mMultipleEmptyLineLimit == -1 ||
							int(curContinuousEmptyCount) <= options.mMultipleEmptyLineLimit) {
						optionalText(options, false, deep, out, "");
					}
					return;
				}

				if (cfgPair.mName.isComment() && cfgPair.mValue.isEmpty()) {
					curContinuousEmptyCount = 0;
					optionalText(options, false, deep, out, "#%s", cfgPair.mName.mText.c_str());
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
					bool isTemplate = false;
					if (options.mShowAdditionalTemplateInfosAtObjectName) {
						isTemplate = changeNameIfTemplateObject(cfgPair, name);
					}
					insertValue(cfgPair.mValue, true, name, index, indexStack,
							options, deep, out, curContinuousEmptyCount, isTemplate);
					return;
				}

				bool forceOutput = (options.mUseSearchMode && options.mSearchName == name);
				curContinuousEmptyCount = 0;
				if (cfgPair.mValue.isEmpty()) {
					optionalText(options, forceOutput, deep, out, "%s", name.c_str());
				}
				else {
					optionalText(options, forceOutput, deep, out, "%s = %s", name.c_str(),
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
		const Value &cfgValue, const GuiRenderOptions& options)
{
	if (!internalLabelId.empty()) {
		ImGui::PushID(internalLabelId.c_str());
	}
	std::vector<size_t> indexStack;
	unsigned int curContinuousEmptyCount = 0;
	unsigned int deep = 0;
	std::ostream* out = nullptr;
	insertValue(cfgValue, false, "", 0,
			options.mUseSearchMode ? &indexStack : nullptr,
			options, deep, out, curContinuousEmptyCount);
	if (!internalLabelId.empty()) {
		ImGui::PopID();
	}
}

void cfg::gui::nameValuePairAsImguiTree(const std::string& internalLabelId,
		const NameValuePair &cfgPair, const GuiRenderOptions& options)
{
	if (!internalLabelId.empty()) {
		ImGui::PushID(internalLabelId.c_str());
	}
	std::vector<size_t> indexStack;
	unsigned int curContinuousEmptyCount = 0;
	unsigned int deep = 0;
	std::ostream* out = nullptr;
	insertNameValuePair(cfgPair, 0,
			options.mUseSearchMode ? &indexStack : nullptr,
			options, deep, out, curContinuousEmptyCount);
	if (!internalLabelId.empty()) {
		ImGui::PopID();
	}
}

void cfg::gui::valueToStream(std::ostream& out, const Value& cfgValue,
		const GuiRenderOptions& options)
{
	unsigned int curContinuousEmptyCount = 0;
	unsigned int deep = 0;
	insertValue(cfgValue, false, "", 0, nullptr,
			options, deep, &out, curContinuousEmptyCount);
}

void cfg::gui::nameValuePairToStream(std::ostream& out,
		const NameValuePair& cfgPair, const GuiRenderOptions& options)
{
	unsigned int curContinuousEmptyCount = 0;
	unsigned int deep = 0;
	insertNameValuePair(cfgPair, 0, nullptr,
			options, deep, &out, curContinuousEmptyCount);
}

void cfg::gui::valueAsImguiTextEx(const Value& cfgValue,
		const GuiRenderOptions& options)
{
	std::stringstream ss;
	valueToStream(ss, cfgValue, options);
	std::string line;
	while (std::getline(ss, line)) {
		ImGui::Text("%s", line.c_str());
	}
}

void cfg::gui::nameValuePairAsImguiTextEx(const NameValuePair& cfgPair,
		const GuiRenderOptions& options)
{
	std::stringstream ss;
	nameValuePairToStream(ss, cfgPair, options);
	std::string line;
	while (std::getline(ss, line)) {
		ImGui::Text("%s", line.c_str());
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
		std::stringstream ss;
		const CfgTemplate& cfgTemplate = entry.second;
		for (const std::string& param : cfgTemplate.getParameters()) {
			ss << " " << param;
		}
		if (ImGui::TreeNode(strId.c_str(), "%-30s %s", entry.first.c_str(),
				ss.str().c_str())) {
			intentText("parameter count: %zu", cfgTemplate.getParameterCount());
			intentText("parameters:%s", ss.str().c_str());
			const std::vector<NameValuePair>& pairs = cfgTemplate.getPairs();
#if 0
			std::size_t pairCnt = pairs.size();
			intentText("template pair count: %zu", pairCnt);
			for (std::size_t i = 0; i < pairCnt; ++i) {
				std::string tempStrId = "temp-" + std::to_string(i);
				nameValuePairAsImguiTree(tempStrId, pairs[i]);
			}
#else
			ImGui::Indent();
			for (const NameValuePair& nvp : pairs) {
				nameValuePairAsImguiText(nvp);
			}
			ImGui::Unindent();
#endif
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

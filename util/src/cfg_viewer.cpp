#include <cfg_viewer.h>
#include <cfg/file_loader.h>
#include <interpreter/interpreter.h>
#include <imgui.h>
#include <sstream>

cfg::CfgViewer::CfgViewer(const std::shared_ptr<FileLoader>& fileLoader)
		:mFileLoader(fileLoader)
{
	mCfgRenderOptions.mMultipleEmptyLineLimit = 0; // --> no empty lines
	mCfgRenderOptions.mImGuiTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
}

bool cfg::CfgViewer::load(const std::string& filename)
{
	mFilename = filename;
	return reload();
}

bool cfg::CfgViewer::reload()
{
	unload();

	++mLoadCount;

	std::string outFullFilename;
	mFileLoader->reset();
	if (!mFileLoader->loadAndPush(mValueOriginal, outFullFilename, mFilename,
			mInclEmptyLines, mInclComments, mErrMsg)) {
		mLoadStatus = LoadStatus::ERR_ORIGINAL_FILE;
		return false;
	}

	mFileLoader->reset();
	if (!cfg::inc::loadAndIncludeFiles(mValueIncluded, mIncludedFiles, mFilename, *mFileLoader,
			"include", mInclEmptyLines, mInclComments, mWithFileBuffering,
			mErrMsg)) {
		mLoadStatus = LoadStatus::ERR_INCLUDED;
		return false;
	}
	mValueWithoutTemplates = mValueIncluded;
	if (!cfg::cfgtemp::addTemplates(mTemplateMap, mValueWithoutTemplates, true, "template",
			mErrMsg)) {
		mLoadStatus = LoadStatus::ERR_ADD_TEMPLATES;
		return false;
	}
	mValueTemplatesApplied = mValueWithoutTemplates;
	if (!cfg::cfgtemp::useTemplates(mTemplateMap, mValueTemplatesApplied,
			"use-template", true, false, mErrMsg)) {
		mLoadStatus = LoadStatus::ERR_USE_TEMPLATES;
		return false;
	}

	Value value = mValueTemplatesApplied;

	if (!cfg::cfgtr::addTranslations(mTranslationsMap, value, true,
			"translations", mErrMsg)) {
		mLoadStatus = LoadStatus::ERR_ADD_TRANSLATIONS;
		return false;
	}
	if (mTranslationsMap.find(mTranslationId) == mTranslationsMap.end()) {
		// --> can't find current translation id
		// --> reset to a new default
		if (mTranslationsMap.empty()) {
			mTranslationId.clear();
		}
		else {
			mTranslationId = mTranslationsMap.begin()->first;
		}
	}
	if (mApplyTranslations) {
		if (!mTranslationsMap.empty()) {
			if (!cfg::cfgtr::useTranslations(mTranslationsMap,
					mTranslationId, "tr(", true, value, mErrMsg)) {
				mLoadStatus = LoadStatus::ERR_USE_TRANSLATIONS;
				return false;
			}
		}
	}

	if (!cfg::cfgtr::addTranslations(mProfilesMap, value, true,
			"profiles", mErrMsg)) {
		mLoadStatus = LoadStatus::ERR_ADD_PROFILES;
		return false;
	}
	if (mProfilesMap.find(mProfileId) == mProfilesMap.end()) {
		// --> can't find current profile id
		// --> reset to a new default
		if (mProfilesMap.empty()) {
			mProfileId.clear();
		}
		else {
			mProfileId = mProfilesMap.begin()->first;
		}
	}
	if (mApplyProfiles) {
		if (!mProfilesMap.empty()) {
			if (!cfg::cfgtr::useTranslations(mProfilesMap,
					mProfileId, "pr(", true, value, mErrMsg)) {
				mLoadStatus = LoadStatus::ERR_USE_PROFILES;
				return false;
			}
		}
	}

	if (!cfg::cfgtr::addVariables(mVariablesMap, value, true,
			"variables", mErrMsg)) {
		mLoadStatus = LoadStatus::ERR_ADD_VARIABLES;
		return false;
	}
	if (mApplyVariables) {
		if (!mVariablesMap.empty()) {
			if (!cfg::cfgtr::useTranslations(mVariablesMap,
					"", "$(", true, value, mErrMsg)) {
				mLoadStatus = LoadStatus::ERR_USE_VARIABLES;
				return false;
			}
		}
	}

	mValueBeforeInterpreter = value;
	mValueAfterInterpreter = mValueBeforeInterpreter;
	std::stringstream errStream;
	if (cfg::interpreter::interpretAndReplace(mValueAfterInterpreter,
			mAllowInterpretationWithQuotes,
			mAllowArrayElementInterpretation,
			mAllowNameInterpretation,
			mAllowValueInterpretation, errStream) == -1) {
		mErrMsg = errStream.str();
		return false;
	}

	mLoadStatus = LoadStatus::LOADED_SUCCESSFUL;
	return true;
}

void cfg::CfgViewer::unload()
{
	mFileLoader->reset();
	mLoadStatus = LoadStatus::UNLOADED;
	mErrMsg.clear();
	mValueOriginal.clear();
	mValueIncluded.clear();
	mIncludedFiles.clear();
	mValueWithoutTemplates.clear();
	mTemplateMap.clear();
	mValueTemplatesApplied.clear();
	mTranslationsMap.clear();
	mProfilesMap.clear();
	mVariablesMap.clear();
	mValueBeforeInterpreter.clear();
	mValueAfterInterpreter.clear();
}

void cfg::CfgViewer::render(unsigned int resWidth, unsigned int resHeight)
{
	if (!mShowCfgViewer) {
		return;
	}

	if (mAutoResize) {
		mAutoResize = false;
		float marginLeft = 300.0f;
		float marginTop = 50.0f;
		float marginRight = 50.0f;
		float marginBottom = 50.0f;
		ImVec2 window_pos(marginLeft, marginTop);
		ImVec2 window_size(resWidth - marginLeft - marginRight, resHeight - marginTop - marginBottom);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
	}
	if (!ImGui::Begin("CfgViewer", &mShowCfgViewer)) {
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	if (ImGui::Button("Reload##reload")) {
		reload();
	}

	gui::intentText("Filename: %s", mFilename.empty() ? "(empty)" : mFilename.c_str());
	gui::intentText("Load count: %u", mLoadCount);
	gui::intentText("Load status: %s", getLoadStatusAsString(mLoadStatus));
	gui::intentText("Error msg: %s", mErrMsg.c_str());
	if (ImGui::TreeNode("cfg-viewer-options", "Load options")) {
		ImGui::Indent();
		ImGui::Text("Reload necessary:");
		ImGui::Checkbox("Apply translations", &mApplyTranslations);
		ImGui::Checkbox("Apply profiles", &mApplyProfiles);
		ImGui::Checkbox("Apply variables", &mApplyVariables);
		ImGui::Checkbox("Incl empty lines", &mInclEmptyLines);
		ImGui::Checkbox("Incl comments", &mInclComments);
		ImGui::Checkbox("With file buffering", &mWithFileBuffering);
		ImGui::Checkbox("Allow interpretation with quotes", &mAllowInterpretationWithQuotes);
		ImGui::Checkbox("Allow array element interpretation", &mAllowArrayElementInterpretation);
		ImGui::Checkbox("Allow name interpretation", &mAllowNameInterpretation);
		ImGui::Checkbox("Allow value interpretation", &mAllowValueInterpretation);
		ImGui::Unindent();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("cfg-gui-render-options", "Render options")) {
		ImGui::Indent();
		ImGui::Checkbox("Show additional template infos",
				&mCfgRenderOptions.mShowAdditionalTemplateInfosAtObjectName);
		int& e = mCfgRenderOptions.mMultipleEmptyLineLimit;
		ImGui::Text("Continuous empty lines:"); ImGui::SameLine();
		ImGui::RadioButton("All", &e, -1); ImGui::SameLine();
		ImGui::RadioButton("None", &e, 0); ImGui::SameLine();
		ImGui::RadioButton("Only one", &e, 1);
		ImGui::Unindent();
		ImGui::TreePop();
	}
	gui::intentText("Translation ID: '%s'", mTranslationId.c_str());
	gui::intentText("Profile ID: '%s'", mProfileId.c_str());

	if (ImGui::CollapsingHeader("File original")) {
		cfg::gui::valueAsImguiTree("file-original", mValueOriginal,
				mCfgRenderOptions);
	}

	std::string includesLabel = "Includes: " +
			std::to_string(mIncludedFiles.size()) + "##includes";
	if (ImGui::CollapsingHeader(includesLabel.c_str())) {
		for (const auto& entry : mIncludedFiles) {
			ImGui::Text("%s - %u", entry.first.c_str(), entry.second);
		}
	}

	if (ImGui::CollapsingHeader("File included")) {
		cfg::gui::valueAsImguiTree("file-included", mValueIncluded,
				mCfgRenderOptions);
	}

	std::string templatesLabel = "Templates: " +
			std::to_string(mTemplateMap.size()) + "##templates";
	if (ImGui::CollapsingHeader(templatesLabel.c_str())) {
		gui::templateMapAsImguiTree("cfg-templates", mTemplateMap);
	}

	if (ImGui::CollapsingHeader("File without templates")) {
		cfg::gui::valueAsImguiTree("file-without-templates",
				mValueWithoutTemplates, mCfgRenderOptions);
	}

	if (ImGui::CollapsingHeader("File with applied templates")) {
		cfg::gui::valueAsImguiTree("file-with-applied-templates",
				mValueTemplatesApplied, mCfgRenderOptions);
	}

	std::string translationsLabel = "Translations: " +
			std::to_string(mTranslationsMap.size()) + "##translations";
	if (ImGui::CollapsingHeader(translationsLabel.c_str())) {
		gui::languageMapAsImguiTree("translations-map", mTranslationsMap);
	}

	std::string profilesLabel = "Profiles: " +
			std::to_string(mProfilesMap.size()) + "##profiles";
	if (ImGui::CollapsingHeader(profilesLabel.c_str())) {
		gui::languageMapAsImguiTree("profiles-map", mProfilesMap);
	}

	std::string variablesLabel = "Variables: " +
			std::to_string(mVariablesMap.size()) + "##variables";
	if (ImGui::CollapsingHeader(variablesLabel.c_str())) {
		gui::languageMapAsImguiTree("variables-map", mVariablesMap);
	}

	if (ImGui::CollapsingHeader("File before interpreter")) {
		cfg::gui::valueAsImguiTree("file-before-interpreter",
				mValueBeforeInterpreter, mCfgRenderOptions);
	}
	if (ImGui::CollapsingHeader("File after interpreter")) {
		cfg::gui::valueAsImguiTree("file-after-interpreter",
				mValueAfterInterpreter, mCfgRenderOptions);
	}
	if (ImGui::CollapsingHeader("Search after interpreter (tree)")) {
		cfg::GuiRenderOptions options = mCfgRenderOptions;
		options.mUseSearchMode = true;
		cfg::gui::valueAsImguiTree("file-search-tree",
				mValueAfterInterpreter, options);
	}
	if (ImGui::CollapsingHeader("Search after interpreter (text)")) {
		cfg::GuiRenderOptions options = mCfgRenderOptions;
		options.mUseSearchMode = true;
		//cfg::gui::valueAsImguiTextEx(mValueAfterInterpreter, options);
		std::stringstream ss;
		cfg::gui::valueToStream(ss, mValueAfterInterpreter, options);
		std::string str = ss.str();
		// copy str to vector because a char* instead of const char* for InputTextMultiline is needed.
		std::vector<char> text(str.begin(), str.end());
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_ReadOnly;
		ImGui::InputTextMultiline("##source", text.data(), text.size(),
				ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 40), flags);
	}

	ImGui::End();
}

const char* cfg::CfgViewer::getLoadStatusAsString(LoadStatus loadStatus)
{
	switch (loadStatus) {
		case LoadStatus::UNLOADED:
			return "UNLOADED";
		case LoadStatus::LOADED_SUCCESSFUL:
			return "LOADED_SUCCESSFUL";
		case LoadStatus::ERR_ORIGINAL_FILE:
			return "ERR_ORIGINAL_FILE";
		case LoadStatus::ERR_INCLUDED:
			return "ERR_INCLUDED";
		case LoadStatus::ERR_ADD_TEMPLATES:
			return "ERR_ADD_TEMPLATES";
		case LoadStatus::ERR_USE_TEMPLATES:
			return "ERR_USE_TEMPLATES";
		case LoadStatus::ERR_ADD_TRANSLATIONS:
			return "ERR_ADD_TRANSLATIONS";
		case LoadStatus::ERR_USE_TRANSLATIONS:
			return "ERR_USE_TRANSLATIONS";
		case LoadStatus::ERR_ADD_PROFILES:
			return "ERR_ADD_PROFILES";
		case LoadStatus::ERR_USE_PROFILES:
			return "ERR_USE_PROFILES";
		case LoadStatus::ERR_ADD_VARIABLES:
			return "ERR_ADD_VARIABLES";
		case LoadStatus::ERR_USE_VARIABLES:
			return "ERR_USE_VARIABLES";
		case LoadStatus::ERR_INTERPRETER:
			return "ERR_INTERPRETER";
	}
	return "unknown-error";
}

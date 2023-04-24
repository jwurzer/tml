#include <cfg_viewer.h>
#include <cfg_gui.h>
#include <cfg/file_loader.h>
#include <interpreter/interpreter.h>
#include <imgui.h>
#include <sstream>

cfg::CfgViewer::CfgViewer(const std::shared_ptr<FileLoader>& fileLoader)
		:mFileLoader(fileLoader)
{
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
		return false;
	}

	mFileLoader->reset();
	if (!cfg::inc::loadAndIncludeFiles(mValueIncluded, mIncludedFiles, mFilename, *mFileLoader,
			"include", mInclEmptyLines, mInclComments, mWithFileBuffering,
			mErrMsg)) {
		return false;
	}
	mValueWithoutTemplates = mValueIncluded;
	if (!cfg::cfgtemp::addTemplates(mTemplateMap, mValueWithoutTemplates, true, "template",
			mErrMsg)) {
		return false;
	}
	mValueTemplatesApplied = mValueWithoutTemplates;
	if (!cfg::cfgtemp::useTemplates(mTemplateMap, mValueTemplatesApplied,
			"use-template", true, false, mErrMsg)) {
		return false;
	}

	Value value = mValueTemplatesApplied;

	if (!cfg::cfgtr::addTranslations(mTranslationsMap, value, true,
			"translations", mErrMsg)) {
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
				return false;
			}
		}
	}

	if (!cfg::cfgtr::addTranslations(mProfilesMap, value, true,
			"profiles", mErrMsg)) {
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
				return false;
			}
		}
	}

	if (!cfg::cfgtr::addVariables(mVariablesMap, value, true,
			"variables", mErrMsg)) {
		return false;
	}
	if (mApplyVariables) {
		if (!mVariablesMap.empty()) {
			if (!cfg::cfgtr::useTranslations(mVariablesMap,
					"", "$(", true, value, mErrMsg)) {
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

	mSuccessfulLoaded = true;
	return true;
}

void cfg::CfgViewer::unload()
{
	mFileLoader->reset();
	mSuccessfulLoaded = false;
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
	gui::intentText("Loaded successful: %s", mSuccessfulLoaded ? "true" : "false");
	gui::intentText("Error msg: %s", mErrMsg.c_str());

	ImGui::Checkbox("Apply translations", &mApplyTranslations);
	ImGui::Checkbox("Apply profiles", &mApplyProfiles);
	ImGui::Checkbox("Apply variables", &mApplyVariables);
	gui::intentText("Translation ID: '%s'", mTranslationId.c_str());
	gui::intentText("Profile ID: '%s'", mProfileId.c_str());

	if (ImGui::CollapsingHeader("File original")) {
		cfg::gui::valueAsImguiTree("file-original", mValueOriginal,
				mAllowMultipleEmptyLines);
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
				mAllowMultipleEmptyLines);
	}

	std::string templatesLabel = "Templates: " +
			std::to_string(mTemplateMap.size()) + "##templates";
	if (ImGui::CollapsingHeader(templatesLabel.c_str())) {
		gui::templateMapAsImguiTree("cfg-templates", mTemplateMap);
	}

	if (ImGui::CollapsingHeader("File without templates")) {
		cfg::gui::valueAsImguiTree("file-without-templates",
				mValueWithoutTemplates,
				mAllowMultipleEmptyLines);
	}

	if (ImGui::CollapsingHeader("File with applied templates")) {
		cfg::gui::valueAsImguiTree("file-with-applied-templates",
				mValueTemplatesApplied,
				mAllowMultipleEmptyLines);
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
				mValueBeforeInterpreter,
				mAllowMultipleEmptyLines);
	}
	if (ImGui::CollapsingHeader("File after interpreter")) {
		cfg::gui::valueAsImguiTree("file-after-interpreter",
				mValueAfterInterpreter,
				mAllowMultipleEmptyLines);
	}

	ImGui::End();
}

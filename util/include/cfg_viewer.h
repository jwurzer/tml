#ifndef UTIL_CFG_VIEWER_H
#define UTIL_CFG_VIEWER_H

#include <cfg/export.h>
#include <cfg/cfg.h>
#include <cfg/cfg_include.h>
#include <cfg/cfg_template.h>
#include <cfg/cfg_translation.h>
#include <memory>

namespace cfg
{
	class FileLoader;

	class CFG_API CfgViewer
	{
	public:
		CfgViewer(const std::shared_ptr<FileLoader>& fileLoader);

		bool load(const std::string& filename);
		bool reload();
		void unload();

		void setVisible() { mShowCfgViewer = true; }
		void render(unsigned int resWidth, unsigned int resHeight);
	private:
		std::shared_ptr<FileLoader> mFileLoader;
		bool mShowCfgViewer = false;
		bool mAutoResize = true;
		bool mApplyTranslations = true;
		bool mApplyProfiles = true;
		bool mApplyVariables = true;
		std::string mTranslationId;
		std::string mProfileId;

		bool mAllowMultipleEmptyLines = false;

		bool mInclEmptyLines = true;
		bool mInclComments = true;
		bool mWithFileBuffering = true;

		bool mAllowInterpretationWithQuotes = false;
		bool mAllowArrayElementInterpretation = true;
		bool mAllowNameInterpretation = true;
		bool mAllowValueInterpretation = true;

		std::string mFilename;
		unsigned int mLoadCount = 0;
		bool mSuccessfulLoaded = false;
		std::string mErrMsg;

		cfg::Value mValueOriginal;
		cfg::Value mValueIncluded;
		cfg::inc::TFileMap mIncludedFiles;
		cfg::Value mValueWithoutTemplates;
		cfg::cfgtemp::TemplateMap mTemplateMap;
		cfg::Value mValueTemplatesApplied;
		cfg::cfgtr::LanguageMap mTranslationsMap;
		cfg::cfgtr::LanguageMap mProfilesMap;
		cfg::cfgtr::LanguageMap mVariablesMap;
		cfg::Value mValueBeforeInterpreter;
		cfg::Value mValueAfterInterpreter;
	};
}

#endif

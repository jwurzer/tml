#ifndef CFG_CFG_TRANSLATION_H
#define CFG_CFG_TRANSLATION_H

#include <cfg/export.h>
#include <cfg/cfg.h>
#include <map>

namespace cfg
{
	class CFG_API CfgTranslation
	{
	public:
		std::string mTranslationId;
		Value mValue;
	};

	namespace cfgtr
	{
		typedef std::map<std::string /* translation id */, CfgTranslation> TranslationMap;
		/**
		 * language id is e.g. EN for english, DE for german etc.
		 */
		typedef std::map<std::string /* language id */, TranslationMap> LanguageMap;

		/**
		 * Apply all translations. Load translations with addTranslations()
		 * and use the translations with useTranslations().
		 * The translations object with its translations is removed.
		 *
		 * @param cfgValue This cfg value must also include the translations object.
		 *        Also the other name-value-pairs with the translations should
		 *        be included.
		 * @param translationsKeyword name of the translations object.
		 *        e.g. "translations"
		 * @param replaceKeyword keyword prefix for translation-id. e.g. "tr("
		 * @param languageId Define which languageId should be used.
		 *        If empty, first languageId is used. The used
		 *        language id is stored in this case into this parameter.
		 * @param outErrorMsg
		 * @return True for success. Otherwise false.
		 */
		CFG_API
		bool applyTranslations(Value& cfgValue,
				const std::string& translationsKeyword,
				const std::string& replaceKeyword,
				std::string& languageId, std::string& outErrorMsg);

		/**
		 * Apply all variables. Load variables with addVariables()
		 * and use the variables with useTranslations() with languageId "".
		 * The variables object with its variables is removed.
		 *
		 * @param cfgValue This cfg value must also include the variables object.
		 *        Also the other name-value-pairs with the variables should
		 *        be included.
		 * @param variablesKeyword name of the variables object.
		 *        e.g. "variables"
		 * @param replaceKeyword keyword prefix for variable-id. e.g. "$("
		 * @param outErrorMsg
		 * @return True for success. Otherwise false.
		 */
		CFG_API
		bool applyVariables(Value& cfgValue,
				const std::string& variablesKeyword,
				const std::string& replaceKeyword,
				std::string& outErrorMsg);

		CFG_API
		bool addTranslations(LanguageMap& languageMap,
				Value& cfgValue, bool removeTranslationsFromCfgValue,
				const std::string& translationsKeyword,
				std::string& outErrorMsg);

		CFG_API
		bool addVariables(LanguageMap& languageMap,
				Value& cfgValue, bool removeVariablesFromCfgValue,
				const std::string& variablesKeyword,
				std::string& outErrorMsg);

		/**
		 * Each name value pair from translations vector represents one translation.
		 * A name value pair for a translation should look like this.
		 * <translation-id> <language-id> = <a-value-like-a-text>
		 * @param languageMap Will be filled by this function.
		 * @param translations Translations for adding to languageMap
		 * @param outErrorMsg Store a error message if an error happened.
		 * @return True for success. False for error.
		 */
		CFG_API
		bool addTranslations(LanguageMap& languageMap,
				const std::vector<NameValuePair>& translations,
				std::string& outErrorMsg);

		CFG_API
		bool addVariables(LanguageMap& languageMap,
				const std::vector<NameValuePair>& translations,
				std::string& outErrorMsg);

		/**
		 * Replace cfgValue and all sub values if a translation id is used.
		 * All translation ids with tr(<translation-id>) are replaced.
		 * tr(<translation-id>) must be a text.
		 * @param languageMap Map for translations
		 * @param languageId Id of language which should be used.
		 * @param replaceKeyword Something like "tr(". Should include a ( at
		 *        the end of the replaceKeyword. Everything which start
		 *        with the replaceKeyword, e.g. "tr(", and end with ")" is
		 *        replaced with the entry of the translationMap.
		 * @param flatArrayVariableIntoArray Example: VAR is 'aa bb cc'.
		 *        If true then 'ww xx $(VAR) yy zz' is replaced to 'ww xx aa bb cc yy zz'.
		 *        If false then 'ww xx $(VAR) yy zz' is replaced to '
		 *        []
		 *            ww
		 *            xx
		 *            aa bb cc
		 *            yy
		 *            zz
		 *        '.
		 * @param cfgValue value which should be replaced
		 * @param outErrorMsg Store a error message if an error happened.
		 * @return True for success. False for error.
		 */
		CFG_API
		bool useTranslations(const LanguageMap& languageMap,
				const std::string& languageId,
				const std::string& replaceKeyword,
				bool flatArrayVariableIntoArray,
				Value& cfgValue,
				std::string& outErrorMsg);

		/**
		 * @param replaceKeyword Something like "tr(". Should include a ( at
		 *        the end of the replaceKeyword. Everything which start
		 *        with the replaceKeyword, e.g. "tr(", and end with ")" is
		 *        replaced with the entry of the translationMap.
		 * @param flatArrayVariableIntoArray Example: VAR is 'aa bb cc'.
		 *        If true then 'ww xx $(VAR) yy zz' is replaced to 'ww xx aa bb cc yy zz'.
		 *        If false then 'ww xx $(VAR) yy zz' is replaced to '
		 *        []
		 *            ww
		 *            xx
		 *            aa bb cc
		 *            yy
		 *            zz
		 *        '.
		 */
		CFG_API
		bool useTranslations(const TranslationMap& translationMap,
				const std::string& replaceKeyword,
				bool flatArrayVariableIntoArray,
				Value& cfgValue, std::string& outErrorMsg);
	}
}

#endif

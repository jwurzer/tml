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

		/**
		 * Replace cfgValue and all sub values if a translation id is used.
		 * All translation ids with tr(<translation-id>) are replaced.
		 * tr(<translation-id>) must be a text.
		 * @param languageMap Map for translations
		 * @param languageId Id of language which should be used.
		 * @param cfgValue value which should be replaced
		 * @param outErrorMsg Store a error message if an error happened.
		 * @return True for success. False for error.
		 */
		CFG_API
		bool useTranslations(const LanguageMap& languageMap,
				const std::string& languageId, Value& cfgValue,
				std::string& outErrorMsg);

		CFG_API
		bool useTranslations(const TranslationMap& translationMap,
				Value& cfgValue, std::string& outErrorMsg);
	}
}

#endif

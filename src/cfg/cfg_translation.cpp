#include <cfg/cfg_translation.h>
#include <sstream>

#define MAX_RECURSIVE_DEEP 50

namespace cfg
{
namespace cfgtr
{
namespace
{
	/**
	 *
	 * @param translationMap
	 * @param replaceKeyword
	 * @param currentRecursiveReplaceDeep Is needed to limit to a max recursive deep.
	 *        This has nothing to do with the current deep of the
	 *        "value tree". This deep value is only for the recursive
	 *        replacement of translations.
	 * @param flatArrayVariableIntoArray Example: VAR is 'aa bb cc'.
	 * If true then 'ww xx $(VAR) yy zz' is replaced to 'ww xx aa bb cc yy zz'.
	 * If false then 'ww xx $(VAR) yy zz' is replaced to '
	 * []
	 *     ww
	 *     xx
	 *     aa bb cc
	 *     yy
	 *     zz
	 * '.
	 * @param cfgValue
	 * @param outErrorMsg
	 * @return
	 */
	bool replaceTranslationIds(const TranslationMap& translationMap,
			const std::string& replaceKeyword,
			int currentRecursiveReplaceDeep,
			bool flatArrayVariableIntoArray,
			Value& cfgValue,
			std::string& outErrorMsg)
	{
		if (currentRecursiveReplaceDeep > MAX_RECURSIVE_DEEP) {
			outErrorMsg = cfgValue.getFilenameAndPosition() +
					": Reach max recursive deep for translation replacement! (deep " +
					std::to_string(currentRecursiveReplaceDeep) + ")";
			return false;
		}
		if (cfgValue.isText()) {
			if (cfgValue.mText.size() < 4) {
				return true;
			}
			if (cfgValue.mText.compare(0, replaceKeyword.size(),
					replaceKeyword) != 0 ||
					cfgValue.mText.back() != ')') {
				return true;
			}
			std::string translationId = cfgValue.mText.substr(
					replaceKeyword.size(),
					cfgValue.mText.length() - (replaceKeyword.size() + 1));
			const TranslationMap::const_iterator it = translationMap.find(
					translationId);
			if (it == translationMap.end()) {
				outErrorMsg = cfgValue.getFilenameAndPosition() +
						": Can't find translation id '" + translationId + "'";
				return false;
			}
			// copy the value to replace the translation holder with the correct value.
			cfgValue = it->second.mValue;
			// the replaced translation can also have a translations as value
			// --> call replaceTranslationIds() for the replaced translation
			if (!replaceTranslationIds(translationMap, replaceKeyword,
					currentRecursiveReplaceDeep + 1,
					flatArrayVariableIntoArray,
					cfgValue, outErrorMsg)) {
				return false;
			}
			return true;
		}

		if (cfgValue.isArray()) {
			std::size_t count = cfgValue.mArray.size();
			for (std::size_t i = 0; i < count; ++i) {
				Value& v = cfgValue.mArray[i];
				bool isArrayBefore = v.isArray();
				// No +1 for currentRecursiveReplaceDeep because its
				// no new recursive replacement.
				// Its not a replacement of a replacement.
				// Don't use 0 for currentRecursiveReplaceDeep to check
				// indirect recursive replacements.
				if (!replaceTranslationIds(translationMap, replaceKeyword,
						currentRecursiveReplaceDeep, flatArrayVariableIntoArray,
						v, outErrorMsg)) {
					return false;
				}
				if (flatArrayVariableIntoArray && !isArrayBefore && v.isArray()) {
					Value tmp = std::move(v);
					std::size_t insertCount = tmp.mArray.size();
					cfgValue.mArray.erase(cfgValue.mArray.begin() + i);
					cfgValue.mArray.insert(cfgValue.mArray.begin() + i, tmp.mArray.begin(), tmp.mArray.end());
					// integer underflow can happen for insertCount - 1 but is ok.
					// If insertCount is 0 then an underflow happened.
					// But ++i from the for() loop reverses this underflow.
					// Also if i is 0.
					// --> has no bad effect.
					i += insertCount - 1;
					// after erase and insert the variable count must be updated!
					count = cfgValue.mArray.size();
				}
			}
			return true;
		}

		if (cfgValue.isObject()) {
			for (NameValuePair& nv : cfgValue.mObject) {
				// No +1 for currentRecursiveReplaceDeep because its
				// no new recursive replacement.
				// Its not a replacement of a replacement.
				// Don't use 0 for currentRecursiveReplaceDeep to check
				// indirect recursive replacements.
				// Now apply the translations for the name and value of each object.
				if (!replaceTranslationIds(translationMap, replaceKeyword,
						currentRecursiveReplaceDeep,
						flatArrayVariableIntoArray,
						nv.mName, outErrorMsg)) {
					return false;
				}
				if (!replaceTranslationIds(translationMap, replaceKeyword,
						currentRecursiveReplaceDeep,
						flatArrayVariableIntoArray,
						nv.mValue, outErrorMsg)) {
					return false;
				}
			}
			return true;
		}

		return true;
	}
}
}
}

bool cfg::cfgtr::applyTranslations(Value& cfgValue,
		const std::string& translationsKeyword,
		const std::string& replaceKeyword,
		std::string& languageId, std::string& outErrorMsg)
{
	LanguageMap languageMap;
	if (!addTranslations(languageMap, cfgValue, true,
			translationsKeyword, outErrorMsg)) {
		return false;
	}
	if (languageMap.empty()) {
		// --> nothing to do
		return true;
	}
	if (languageId.empty()) {
		languageId = languageMap.begin()->first;
	}
	return useTranslations(languageMap, languageId, replaceKeyword,
			true, cfgValue, outErrorMsg);
}

bool cfg::cfgtr::applyVariables(Value& cfgValue,
		const std::string& variablesKeyword,
		const std::string& replaceKeyword,
		std::string& outErrorMsg)
{
	LanguageMap languageMap;
	if (!addVariables(languageMap, cfgValue, true,
			variablesKeyword, outErrorMsg)) {
		return false;
	}
	if (languageMap.empty()) {
		// --> nothing to do
		return true;
	}
	return useTranslations(languageMap, "", replaceKeyword,
			true, cfgValue, outErrorMsg);
}

bool cfg::cfgtr::addTranslations(LanguageMap& languageMap,
		Value& cfgValue, bool removeTranslationsFromCfgValue,
		const std::string& translationsKeyword,
		std::string& outErrorMsg)
{
	if (!cfgValue.isObject()) {
		outErrorMsg = "is no object";
		return false;
	}
	std::vector<NameValuePair>& pairs = cfgValue.mObject;
	std::size_t cnt = pairs.size();
	for (std::size_t i = 0; i < cnt; ++i) {
		NameValuePair& nvp = pairs[i];
		if (nvp.mName.mText == translationsKeyword) {
			if (!nvp.mValue.isObject()) {
				outErrorMsg = nvp.mValue.getFilenameAndPosition() +
						": is no object";
				return false;
			}
			std::vector<NameValuePair>& pairsFromTranslations = nvp.mValue.mObject;
			if (!addTranslations(languageMap, pairsFromTranslations, outErrorMsg)) {
				return false;
			}
			if (removeTranslationsFromCfgValue) {
				pairs.erase(pairs.begin() + i);
				--i;
				--cnt;
			}
		}
	}
	return true;
}

bool cfg::cfgtr::addVariables(LanguageMap& languageMap,
		Value& cfgValue, bool removeVariablesFromCfgValue,
		const std::string& variablesKeyword,
		std::string& outErrorMsg)
{
	if (!cfgValue.isObject()) {
		outErrorMsg = "is no object";
		return false;
	}
	std::vector<NameValuePair>& pairs = cfgValue.mObject;
	std::size_t cnt = pairs.size();
	for (std::size_t i = 0; i < cnt; ++i) {
		NameValuePair& nvp = pairs[i];
		if (nvp.mName.mText == variablesKeyword) {
			if (!nvp.mValue.isObject()) {
				outErrorMsg = nvp.mValue.getFilenameAndPosition() +
						": is no object";
				return false;
			}
			std::vector<NameValuePair>& pairsFromTranslations = nvp.mValue.mObject;
			if (!addVariables(languageMap, pairsFromTranslations, outErrorMsg)) {
				return false;
			}
			if (removeVariablesFromCfgValue) {
				pairs.erase(pairs.begin() + i);
				--i;
				--cnt;
			}
		}
	}
	return true;
}

bool cfg::cfgtr::addTranslations(LanguageMap& languageMap,
		const std::vector<NameValuePair>& translations,
		std::string& outErrorMsg)
{
	for (const NameValuePair& nv : translations) {
		if (nv.isEmptyOrComment()) {
			continue;
		}
		if (!nv.mName.isArray()) {
			outErrorMsg = nv.mName.getFilenameAndPosition() + ": is not an array.";
			return false;
		}
		if (nv.mName.mArray.size() != 2) {
			outErrorMsg = nv.mName.getFilenameAndPosition() + ": must have a size of 2.";
			return false;
		}
		if (!nv.mName.mArray[0].isText()) {
			outErrorMsg = nv.mName.getFilenameAndPosition() + ": first element must be a text.";
			return false;
		}
		if (!nv.mName.mArray[1].isText()) {
			outErrorMsg = nv.mName.getFilenameAndPosition() + ": second element must be a text.";
			return false;
		}
		const std::string& translationId = nv.mName.mArray[0].mText;
		const std::string& languageId = nv.mName.mArray[1].mText;
		TranslationMap& translationMap = languageMap[languageId];
		if (translationMap.find(translationId) != translationMap.end()) {
			outErrorMsg = nv.mName.getFilenameAndPosition() +
					": translation id '" + translationId + "' already used.";
			return false;
		}
		CfgTranslation& translation = translationMap[translationId];
		translation.mTranslationId = translationId;
		translation.mValue = nv.mValue;
	}
	return true;
}

bool cfg::cfgtr::addVariables(LanguageMap& languageMap,
		const std::vector<NameValuePair>& translations,
		std::string& outErrorMsg)
{
	const std::string languageId = "";
	TranslationMap& translationMap = languageMap[languageId];

	for (const NameValuePair& nv : translations) {
		if (nv.isEmptyOrComment()) {
			continue;
		}
		if (!nv.mName.isText()) {
			outErrorMsg = nv.mName.getFilenameAndPosition() + ": is not an text.";
			return false;
		}
		const std::string& translationId = nv.mName.mText;
		if (translationMap.find(translationId) != translationMap.end()) {
			outErrorMsg = nv.mName.getFilenameAndPosition() +
					": translation id '" + translationId + "' already used.";
			return false;
		}
		CfgTranslation& translation = translationMap[translationId];
		translation.mTranslationId = translationId;
		translation.mValue = nv.mValue;
	}
	return true;
}

bool cfg::cfgtr::useTranslations(const LanguageMap& languageMap,
		const std::string& languageId, const std::string& replaceKeyword,
		bool flatArrayVariableIntoArray, Value& cfgValue,
		std::string& outErrorMsg)
{
	LanguageMap::const_iterator it = languageMap.find(languageId);
	if (it == languageMap.end()) {
		outErrorMsg = "Can't find translation map for language id '" + languageId + "'";
		return false;
	}
	return useTranslations(it->second, replaceKeyword,
			flatArrayVariableIntoArray, cfgValue, outErrorMsg);
}

bool cfg::cfgtr::useTranslations(const TranslationMap& translationMap,
		const std::string& replaceKeyword, bool flatArrayVariableIntoArray,
		Value& cfgValue, std::string& outErrorMsg)
{
	return replaceTranslationIds(translationMap, replaceKeyword, 0,
			flatArrayVariableIntoArray, cfgValue, outErrorMsg);
}

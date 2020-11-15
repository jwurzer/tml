#include <cfg/cfg_translation.h>
#include <sstream>

bool cfg::cfgtr::addTranslations(LanguageMap& languageMap,
		const std::vector<NameValuePair>& translations,
		std::string& outErrorMsg)
{
	for (const NameValuePair& nv : translations) {
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

bool cfg::cfgtr::useTranslations(const LanguageMap& languageMap,
		const std::string& languageId, Value& cfgValue,
		std::string& outErrorMsg)
{
	LanguageMap::const_iterator it = languageMap.find(languageId);
	if (it == languageMap.end()) {
		outErrorMsg = "Can't find translation map for language id '" + languageId + "'";
		return false;
	}
	return useTranslations(it->second, cfgValue, outErrorMsg);
}

bool cfg::cfgtr::useTranslations(const TranslationMap& translationMap,
		Value& cfgValue, std::string& outErrorMsg)
{
	if (cfgValue.isText()) {
		if (cfgValue.mText.size() < 4) {
			return true;
		}
		if (cfgValue.mText.compare(0, 3, "tr(") != 0 ||
				cfgValue.mText.back() != ')') {
			return true;
		}
		std::string translationId = cfgValue.mText.substr(3, cfgValue.mText.length() - 4);
		const TranslationMap::const_iterator it = translationMap.find(translationId);
		if (it == translationMap.end()) {
			outErrorMsg = cfgValue.getFilenameAndPosition() +
					": Can't find translation id '" + translationId + "'";
			return false;
		}
		cfgValue = it->second.mValue;
		return true;
	}

	if (cfgValue.isArray()) {
		for (Value& v : cfgValue.mArray) {
			if (!useTranslations(translationMap, v, outErrorMsg)) {
				return false;
			}
		}
		return true;
	}

	if (cfgValue.isObject()) {
		for (NameValuePair& nv : cfgValue.mObject) {
			if (!useTranslations(translationMap, nv.mName, outErrorMsg)) {
				return false;
			}
			if (!useTranslations(translationMap, nv.mValue, outErrorMsg)) {
				return false;
			}
		}
		return true;
	}

	return true;
}

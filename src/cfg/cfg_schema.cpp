#include <cfg/cfg_schema.h>

namespace cfg {
	namespace schema {
		namespace {

			bool keywordCheck(const Value* values, unsigned int cnt, const std::string& keyword, std::string& outErrorMsg)
			{
				if (!values || !cnt) {
					outErrorMsg = "keyword " + keyword + ": wrong parameters (internal)";
					return false;
				}
				if (values[0].mText != keyword) {
					outErrorMsg = values[0].getFilenameAndPosition() + ": wrong keyword for " + keyword;
					return false;
				}
				if (cnt == 1) {
					outErrorMsg = values[0].getFilenameAndPosition() + ": no args for keyword " + keyword;
					return false;
				}
				return true;
			}

			bool handleKeywordType(const Value* values, unsigned int cnt, NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				if (!keywordCheck(values, cnt, "type", outErrorMsg)) {
					return false;
				}
				for (unsigned int i = 1; i < cnt; ++i) {
					const std::string& type = values[i].mText;
					if (type == "none") {
						// is 'none' useful ??? is for none, nothing, empty (empty line)
						nvfs.mAllowedTypes |= SelectRule::ALLOW_NONE;
					}
					else if (type == "null" || values[i].isNull()) {
						nvfs.mAllowedTypes |= SelectRule::ALLOW_NULL;
					}
					else if (type == "boolean") {
						nvfs.mAllowedTypes |= SelectRule::ALLOW_BOOL;
					}
					else if (type == "float") {
						nvfs.mAllowedTypes |= SelectRule::ALLOW_FLOAT;
					}
					else if (type == "integer") {
						nvfs.mAllowedTypes |= SelectRule::ALLOW_INT;
					}
					else if (type == "number") {
						// number is a group type allows float and integer
						nvfs.mAllowedTypes |= SelectRule::ALLOW_NUMBER;
					}
					else if (type == "string") {
						// or "text" ???
						nvfs.mAllowedTypes |= SelectRule::ALLOW_TEXT;
					}
					//else if (type == "comment") {
					//	// useful ???
					//	nvfs.mAllowedTypes |= SelectRule::ALLOW_COMMENT;
					//}
					else if (type == "array") {
						nvfs.mAllowedTypes |= SelectRule::ALLOW_ARRAY;
					}
					else if (type == "object") {
						nvfs.mAllowedTypes |= SelectRule::ALLOW_OBJECT;
					}
					else {
						outErrorMsg = values[i].getFilenameAndPosition() + ": '" + type + "' is not supported as type";
						return false;
					}
				}
				return true;
			}

			bool handleKeywordValue(const Value* values, unsigned int cnt, NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				if (!keywordCheck(values, cnt, "value", outErrorMsg)) {
					return false;
				}
				nvfs.mValueIsUsed = true;
				if (cnt == 2) {
					nvfs.mValue = values[1];
					return true;
				}
				nvfs.mValue.setArray();
				for (unsigned int i = 1; i < cnt; ++i) {
					nvfs.mValue.mArray.push_back(values[i]);
				}
				return true;
			}

			bool handleKeywordDefault(const Value* values, unsigned int cnt, NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				if (!keywordCheck(values, cnt, "default", outErrorMsg)) {
					return false;
				}
				nvfs.mDefaultIsUsed = true;
				if (cnt == 2) {
					nvfs.mDefault = values[1];
					return true;
				}
				nvfs.mDefault.setArray();
				for (unsigned int i = 1; i < cnt; ++i) {
					nvfs.mDefault.mArray.push_back(values[i]);
				}
				return true;
			}

			bool handleKeywordMinOrMax(const Value* values, unsigned int cnt, NVFragmentSchema& nvfs, bool isMin,
					std::string& outErrorMsg)
			{
				std::string keyword = isMin ? "min" : "max";
				if (!keywordCheck(values, cnt, keyword, outErrorMsg)) {
					return false;
				}
				if (cnt != 2) {
					outErrorMsg = values[0].getFilenameAndPosition() + ": keyword " + keyword + " need exactly one argument";
					return false;
				}
				const cfg::Value& cfgMinMax = values[1];
				bool& floatMinMaxUsed = isMin ? nvfs.mFloatMinUsed : nvfs.mFloatMaxUsed;
				bool& intMinMaxUsed = isMin ? nvfs.mIntMinUsed : nvfs.mIntMaxUsed;
				if (floatMinMaxUsed || intMinMaxUsed) {
					outErrorMsg = cfgMinMax.getFilenameAndPosition() + ": " + keyword + " is already used.";
					return false;
				}
				if (cfgMinMax.isFloat()) {
					float& floatMinMax = isMin ? nvfs.mFloatMin : nvfs.mFloatMax;
					floatMinMax = cfgMinMax.mFloatingPoint;
					floatMinMaxUsed = true;
				}
				else if (cfgMinMax.isInteger()) {
					int& intMinMax = isMin ? nvfs.mIntMin : nvfs.mIntMax;
					intMinMax = cfgMinMax.mInteger;
					intMinMaxUsed = true;
				}
				else {
					outErrorMsg = cfgMinMax.getFilenameAndPosition() + ": Wrong value type for " + keyword + ". Must be float or integer";
					return false;
				}
				return true;
			}

			bool handleKeywordMin(const Value* values, unsigned int cnt, NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				return handleKeywordMinOrMax(values, cnt, nvfs, true, outErrorMsg);
			}

			bool handleKeywordMax(const Value* values, unsigned int cnt, NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				return handleKeywordMinOrMax(values, cnt, nvfs, false, outErrorMsg);
			}

			bool addKeywordToNVFragmentSchema(const Value* values, unsigned int cnt, NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				if (!values) {
					outErrorMsg = "null pointer for keyword (internal error)";
					return false;
				}
				if (!cnt) {
					outErrorMsg = "empty array for keyword. count is 0. (internal error)";
					return false;
				}
				if (!values[0].isText()) {
					outErrorMsg = values[0].getFilenameAndPosition() + ": keyword must be a text";
					return false;
				}
				const std::string& keyword = values[0].mText;
				if (cnt == 1) {
					outErrorMsg = values[0].getFilenameAndPosition() + ": No arguments for keyword " + keyword;
					return false;
				}
				if (keyword == "type") {
					if (!handleKeywordType(values, cnt, nvfs, outErrorMsg)) {
						return false;
					}
				}
				else if (keyword == "value") {
					if (!handleKeywordValue(values, cnt, nvfs, outErrorMsg)) {
						return false;
					}
				}
				else if (keyword == "default") {
					if (!handleKeywordDefault(values, cnt, nvfs, outErrorMsg)) {
						return false;
					}
				}
				else if (keyword == "min") {
					if (!handleKeywordMin(values, cnt, nvfs, outErrorMsg)) {
						return false;
					}
				}
				else if (keyword == "max") {
					if (!handleKeywordMax(values, cnt, nvfs, outErrorMsg)) {
						return false;
					}
				}
				else {
					outErrorMsg = values[0].getFilenameAndPosition() + ": keyword '" + keyword + "' is not supported";
					return false;
				}
				//outErrorMsg += " TODO";
				//for (unsigned int i = 0; i < cnt; ++i) {
				//	outErrorMsg += " '" + values[i].mText + "'";
				//}
				return true;
			}

			bool addNVFragmentSchema(const Value* values, unsigned int cnt, NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				if (!values) {
					outErrorMsg = "null pointer (internal error)";
					return false;
				}
				if (!cnt) {
					outErrorMsg = "empty array. count is 0. (internal error)";
					return false;
				}
				unsigned int start = 0;
				for (unsigned int i = 0; i < cnt; ++i) {
					if (values[i].mText == ":") {
						unsigned int length = i - start;
						if (!length) {
							outErrorMsg = values[i].getFilenameAndPosition() + ": no values for validation keyword";
							return false;
						}
						if (!addKeywordToNVFragmentSchema(values + start, length, nvfs, outErrorMsg)) {
							return false;
						}
						// +1 because ":" should not be included for next keyword
						start = i + 1; // --> highest possible value is cnt if last is :
					}
				}
				unsigned int length = cnt - start;
				if (length > 0) {
					if (!addKeywordToNVFragmentSchema(values + start, length, nvfs, outErrorMsg)) {
						return false;
					}
				}
				return true;
			}

			int addNVPSchema(const NameValuePair& nvp, NVPSchema& nvps, std::string& outErrorMsg);

			bool addNVPSchemaToNVFragmentSchemaFromObject(const Value& valObj, NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				if (!valObj.isObject()) {
					outErrorMsg = valObj.getFilenameAndPosition() + ": is no object.";
					return false;
				}
				if (valObj.mObject.empty()) {
					outErrorMsg = valObj.getFilenameAndPosition() + ": empty object is not allowed.";
					return false;
				}
				int addRv = 1;
				for (const NameValuePair& nvp : valObj.mObject) {
					// if addRv is 0 then the previous name-value pair was empty or comment which doesn't create a schema.
					// --> if 0 --> no new entry is necessary. previous can be used.
					if (addRv >= 1) {
						nvfs.mObject.emplace_back();
					}
					addRv = addNVPSchema(nvp, nvfs.mObject.back(), outErrorMsg);
					if (addRv == -1) {
						return -1;
					}
				}
				if (addRv == 0) {
					nvfs.mObject.pop_back();
				}
				return true;
			}

			// return can be 1, 0 or -1. -1 for error. 0 for ignored (empty, comment etc), 1 for added
			int addNVPSchema(const NameValuePair& nvp, NVPSchema& nvps, std::string& outErrorMsg)
			{
				nvps.clear();

				if (nvp.isEmptyOrComment()) {
					return 0;
				}

				if (nvp.mName.isEmpty()) {
					outErrorMsg = nvp.mName.getFilenameAndPosition() + ": name of name-value pair is empty but not value.";
					return -1;
				}

				if (nvp.mName.isArray()) {
					if (!addNVFragmentSchema(nvp.mName.mArray.data(), nvp.mName.mArray.size(), nvps.mName, outErrorMsg)) {
						return -1;
					}
				}
				else if (nvp.mName.isText()) {
					if (!addNVFragmentSchema(&nvp.mName, 1, nvps.mName, outErrorMsg)) {
						return -1;
					}
				}
				else {
					outErrorMsg = nvp.mName.getFilenameAndPosition() + ": wrong syntax for name of name-value pair schema";
					return -1;
				}

				if (nvp.mValue.isEmpty()) {
					// this is ok. name was not empty and value is empty. --> return 1 and not 0 because name was used. 0 only if name and value is empty
					return 1;
				}

				if (nvp.mValue.isArray()) {
					if (!addNVFragmentSchema(nvp.mValue.mArray.data(), nvp.mValue.mArray.size(), nvps.mValue, outErrorMsg)) {
						return -1;
					}
				}
				else if (nvp.mValue.isText()) {
					if (!addNVFragmentSchema(&nvp.mValue, 1, nvps.mValue, outErrorMsg)) {
						return -1;
					}
				}
				else if (nvp.mValue.isObject()) {
					if (!addNVPSchemaToNVFragmentSchemaFromObject(nvp.mValue, nvps.mValue, outErrorMsg)) {
						return -1;
					}
				}
				else {
					outErrorMsg = nvp.mValue.getFilenameAndPosition() + ": wrong syntax for value of name-value pair schema";
					return -1;
				}
				nvps.mValueIsUsed = true;
				return 1;
			}

			bool addNVPSchemaToCfgValue(NameValuePair& outNvp,
					const NVPSchema& nvps, std::string& outErrorMsg);

			bool addNVFragmentSchemaToCfgValue(Value& outCfgValue,
					const NVFragmentSchema& nvfs, std::string& outErrorMsg)
			{
				outCfgValue.clear();
				if (!nvfs.mObject.empty()) {
					outCfgValue.setObject();
					for (const NVPSchema& schema : nvfs.mObject) {
						outCfgValue.mObject.emplace_back();
						if (!addNVPSchemaToCfgValue(outCfgValue.mObject.back(), schema, outErrorMsg)) {
							return false;
						}
					}
					return true;
				}

				if (!nvfs.mArray.empty()) {
					outErrorMsg = outCfgValue.getFilenameAndPosition() + ": TODO: array support";
					return false;
				}

				// --> is no object and no array --> a "normal" fragment schema for name or value

				// keyword: type
				if (nvfs.mAllowedTypes == 0) {
					outErrorMsg = outCfgValue.getFilenameAndPosition() + ": no type specified";
					return false;
				}
				outCfgValue.setArray();
				std::vector<Value>& arr = outCfgValue.mArray;
				arr.emplace_back();
				arr.back().setText("type");
				if (nvfs.mAllowedTypes & SelectRule::ALLOW_NONE) {
					arr.emplace_back();
					arr.back().setText("none");
				}
				if (nvfs.mAllowedTypes & SelectRule::ALLOW_NULL) {
					arr.emplace_back();
					arr.back().setText("null");
				}
				if (nvfs.mAllowedTypes & SelectRule::ALLOW_BOOL) {
					arr.emplace_back();
					arr.back().setText("boolean");
				}
				if ((nvfs.mAllowedTypes & SelectRule::ALLOW_NUMBER) == SelectRule::ALLOW_NUMBER) {
					// == ALLOW_NUMBER is necessary because both bits for float and integer must be set.
					// number is a group type which allows float and integer
					arr.emplace_back();
					arr.back().setText("number");
				}
				else {
					if (nvfs.mAllowedTypes & SelectRule::ALLOW_FLOAT) {
						arr.emplace_back();
						arr.back().setText("float");
					}
					if (nvfs.mAllowedTypes & SelectRule::ALLOW_INT) {
						arr.emplace_back();
						arr.back().setText("integer");
					}
				}
				if (nvfs.mAllowedTypes & SelectRule::ALLOW_TEXT) {
					// or "text" ???
					arr.emplace_back();
					arr.back().setText("string");
				}
				//if (nvfs.mAllowedTypes & SelectRule::ALLOW_COMMENT) { // currently not exist
				//	arr.emplace_back();
				//	arr.back().setText("comment");
				//}
				if (nvfs.mAllowedTypes & SelectRule::ALLOW_ARRAY) {
					arr.emplace_back();
					arr.back().setText("array");
				}
				if (nvfs.mAllowedTypes & SelectRule::ALLOW_OBJECT) {
					arr.emplace_back();
					arr.back().setText("object");
				}

				// keyword: value
				if (nvfs.mValueIsUsed) {
					if (nvfs.mValue.isObject()) {
						outErrorMsg = outCfgValue.getFilenameAndPosition() + ": object is not allowed for keyword value";
						return false;
					}
					arr.emplace_back();
					arr.back().setText(":");
					arr.emplace_back();
					arr.back().setText("value");
					if (nvfs.mValue.isArray()) {
						for (const Value& val : nvfs.mValue.mArray) {
							arr.push_back(val);
						}
					}
					else {
						arr.push_back(nvfs.mValue);
					}
				}

				// keyword: default
				if (nvfs.mDefaultIsUsed) {
					if (nvfs.mDefault.isObject()) {
						outErrorMsg = outCfgValue.getFilenameAndPosition() + ": object is not allowed for keyword value";
						return false;
					}
					arr.emplace_back();
					arr.back().setText(":");
					arr.emplace_back();
					arr.back().setText("default");
					if (nvfs.mDefault.isArray()) {
						for (const Value& val : nvfs.mDefault.mArray) {
							arr.push_back(val);
						}
					}
					else {
						arr.push_back(nvfs.mDefault);
					}
				}

				// keyword: min
				if (nvfs.mFloatMinUsed && nvfs.mIntMinUsed) {
					outErrorMsg = outCfgValue.getFilenameAndPosition() + ": min invalid internal state";
					return false;
				}
				if (nvfs.mFloatMinUsed || nvfs.mIntMinUsed) {
					arr.emplace_back();
					arr.back().setText(":");
					arr.emplace_back();
					arr.back().setText("min");
					arr.emplace_back();
					if (nvfs.mFloatMinUsed) {
						arr.back().setFloatingPoint(nvfs.mFloatMin);
					}
					else { // --> nvfs.mIntMinUsed is true
						arr.back().setInteger(nvfs.mIntMin);
					}
				}

				// keyword: max
				if (nvfs.mFloatMaxUsed && nvfs.mIntMaxUsed) {
					outErrorMsg = outCfgValue.getFilenameAndPosition() + ": max invalid internal state";
					return false;
				}
				if (nvfs.mFloatMaxUsed || nvfs.mIntMaxUsed) {
					arr.emplace_back();
					arr.back().setText(":");
					arr.emplace_back();
					arr.back().setText("max");
					arr.emplace_back();
					if (nvfs.mFloatMaxUsed) {
						arr.back().setFloatingPoint(nvfs.mFloatMax);
					}
					else { // --> nvfs.mIntMaxUsed is true
						arr.back().setInteger(nvfs.mIntMax);
					}
				}

				// all keywords are handled ;-) --> now finish function
				return true;
			}

			bool addNVPSchemaToCfgValue(NameValuePair& outNvp,
					const NVPSchema& nvps, std::string& outErrorMsg)
			{
				outNvp.clear();
				if (!addNVFragmentSchemaToCfgValue(outNvp.mName, nvps.mName, outErrorMsg)) {
					return false;
				}
				if (nvps.mValueIsUsed) {
					if (!addNVFragmentSchemaToCfgValue(outNvp.mValue, nvps.mValue, outErrorMsg)) {
						return false;
					}
				}
				return true;
			}
		}
	}
}

void cfg::NVFragmentSchema::clear()
{
	mAllowedTypes = 0;

	mValueIsUsed = false;
	mValue.clear();
	mDefaultIsUsed = false;
	mDefault.clear();

	mFloatMinUsed = false;
	mFloatMin = 0;
	mFloatMaxUsed = false;
	mFloatMax = 0;

	mIntMinUsed = false;
	mIntMin = 0;
	mIntMaxUsed = false;
	mIntMax = 0;

	mArray.clear();
	mObject.clear();
}

bool cfg::schema::getSchemaFromCfgValue(NVFragmentSchema& outNvfs, const Value& cfgValue,
		std::string& outErrorMsg)
{
	if (!cfgValue.isObject()) {
		outErrorMsg = "no object for schema";
		return false;
	}

	outNvfs.clear();
	return addNVPSchemaToNVFragmentSchemaFromObject(cfgValue, outNvfs, outErrorMsg);
}

bool cfg::schema::getSchemaAsCfgValue(Value& outCfgValue,
		const NVFragmentSchema& nvfs, std::string& outErrorMsg)
{
	return addNVFragmentSchemaToCfgValue(outCfgValue, nvfs, outErrorMsg);
}


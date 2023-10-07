#include <cfg/cfg.h>
#include <cmath>
#include <string.h>

namespace cfg
{
	namespace
	{
		// return -1 for not found or not allowed
		int getRuleIndex(const std::string& ruleName, std::size_t curRuleIndex,
				const SelectRule* rules, std::size_t rulesSize,
				bool allowRandomSequence)
		{
			if (curRuleIndex < rulesSize && ruleName == rules[curRuleIndex].mName) {
				return static_cast<int>(curRuleIndex);
			}
			// first search after the current rule index is also allowed
			// if random sequence is false because the sequence is unchanged.
			// Only some rules can be skipped.
			// If a rule should not be skipped then SelectRule::RULE_MUST_EXIST
			// should be used.
			for (std::size_t i = curRuleIndex + 1; i < rulesSize; i++) {
				if (ruleName == rules[i].mName) {
					return static_cast<int>(i);
				}
			}
			if (!allowRandomSequence) {
				return -1; // no other rule allowed
			}
			// not found --> search before the current rule index
			// The additional check with i < rulesSize is necessary because
			// curRuleIndex can be greater than rulesSize.
			for (std::size_t i = 0; i < curRuleIndex && i < rulesSize; i++) {
				if (ruleName == rules[i].mName) {
					return static_cast<int>(i);
				}
			}
			return -1; // no rule found
		}
	}
}

cfg::Value::Value()
		:mFilename(),
		mLineNumber(-1),
		mOffset(-1),
		mNvpDeep(-1),
		mType(TYPE_NONE),
		mParseBase(0),
		mParseTextWithQuotes(false),
		mBool(false),
		mFloatingPoint(0.0f),
		mInteger(0),
		mText(),
		mArray(),
		mObject()
{
}

cfg::Value::Value(bool boolValue,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
		:mFilename(filename),
		mLineNumber(lineNumber),
		mOffset(offset),
		mNvpDeep(nvpDeep),
		mType(TYPE_BOOL),
		mParseBase(1),
		mParseTextWithQuotes(false),
		mBool(boolValue),
		mFloatingPoint(static_cast<float>(boolValue)),
		mInteger(static_cast<int>(boolValue)),
		mText(),
		mArray(),
		mObject()
{
}

cfg::Value::Value(float floatingPointValue,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
		:mFilename(filename),
		mLineNumber(lineNumber),
		mOffset(offset),
		mNvpDeep(nvpDeep),
		mType(TYPE_FLOAT),
		mParseBase(10),
		mParseTextWithQuotes(false),
		mBool(floatingPointValue >= 0.5f || floatingPointValue <= -0.5f),
		mFloatingPoint(floatingPointValue),
		mInteger(static_cast<int>(floatingPointValue + ((floatingPointValue >= 0.0) ? 0.5f : -0.5f))),
		mText(),
		mArray(),
		mObject()
{
}

cfg::Value::Value(int integerValue, unsigned int parseBase,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
		:mFilename(filename),
		mLineNumber(lineNumber),
		mOffset(offset),
		mNvpDeep(nvpDeep),
		mType(TYPE_INT),
		mParseBase(parseBase),
		mParseTextWithQuotes(false),
		mBool(!!integerValue),
		mFloatingPoint(static_cast<float>(integerValue)),
		mInteger(integerValue),
		mText(),
		mArray(),
		mObject()
{
	if (integerValue == 0 && parseBase == 0) {
		mType = TYPE_NULL;
		mParseBase = 0;
		mBool = false;
		mFloatingPoint = 0.0f;
		mInteger = 0;
	}
}

cfg::Value::Value(const std::string& text,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
		:mFilename(filename),
		mLineNumber(lineNumber),
		mOffset(offset),
		mNvpDeep(nvpDeep),
		mType(TYPE_TEXT),
		mParseBase(0),
		mParseTextWithQuotes(false),
		mBool(false),
		mFloatingPoint(0.0f),
		mInteger(0),
		mText(text),
		mArray(),
		mObject()
{
}

cfg::Value::Value(const std::vector<Value>& array,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
		:mFilename(filename),
		mLineNumber(lineNumber),
		mOffset(offset),
		mNvpDeep(nvpDeep),
		mType(TYPE_ARRAY),
		mParseBase(0),
		mParseTextWithQuotes(false),
		mBool(false),
		mFloatingPoint(0.0f),
		mInteger(0),
		mText(),
		mArray(array),
		mObject()
{
}

cfg::Value::Value(const std::vector<NameValuePair>& object,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
		:mFilename(filename),
		mLineNumber(lineNumber),
		mOffset(offset),
		mNvpDeep(nvpDeep),
		mType(TYPE_OBJECT),
		mParseBase(0),
		mParseTextWithQuotes(false),
		mBool(false),
		mFloatingPoint(0.0f),
		mInteger(0),
		mText(),
		mArray(),
		mObject(object)
{
}


cfg::Value::Value(Value&& other)
		:mFilename(std::move(other.mFilename)),
		mLineNumber(std::move(other.mLineNumber)),
		mOffset(std::move(other.mOffset)),
		mNvpDeep(std::move(other.mNvpDeep)),
		mType(std::move(other.mType)),
		mParseBase(std::move(other.mParseBase)),
		mParseTextWithQuotes(std::move(other.mParseTextWithQuotes)),
		mBool(std::move(other.mBool)),
		mFloatingPoint(std::move(other.mFloatingPoint)),
		mInteger(std::move(other.mInteger)),
		mText(std::move(other.mText)),
		mArray(std::move(other.mArray)),
		mObject(std::move(other.mObject))
{
	other.mLineNumber = -1;
	other.mOffset = -1;
	other.mNvpDeep = -1;
	other.mType = TYPE_NONE;
	other.mParseBase = 0;
	other.mParseTextWithQuotes = false;
	other.mBool = false;
	other.mFloatingPoint = 0.0;
	other.mInteger = 0;
}

cfg::Value& cfg::Value::operator=(Value&& other)
{
	if (this == &other) {
		return *this;
	}
	mFilename = std::move(other.mFilename);
	mLineNumber = std::move(other.mLineNumber);
	mOffset = std::move(other.mOffset);
	mNvpDeep = std::move(other.mNvpDeep);
	mType = std::move(other.mType);
	mParseBase = std::move(other.mParseBase);
	mParseTextWithQuotes = std::move(other.mParseTextWithQuotes);
	mBool = std::move(other.mBool);
	mFloatingPoint = std::move(other.mFloatingPoint);
	mInteger = std::move(other.mInteger);
	mText = std::move(other.mText);
	mArray = std::move(other.mArray);
	mObject = std::move(other.mObject);

	other.mLineNumber = -1;
	other.mOffset = -1;
	other.mNvpDeep = -1;
	other.mType = TYPE_NONE;
	other.mParseBase = 0;
	other.mParseTextWithQuotes = false;
	other.mBool = false;
	other.mFloatingPoint = 0.0;
	other.mInteger = 0;

	return *this;
}

std::string cfg::Value::getFilePosition() const
{
	char str[30] = "";
	if (mLineNumber >= 0 && mOffset >= 0) {
		snprintf(str, 30, ":%d:%d", mLineNumber, mOffset);
	}
	else if (mLineNumber >= 0) {
		snprintf(str, 30, ":%d", mLineNumber);
	}
	else if (mOffset >= 0) {
		snprintf(str, 30, "::%d", mOffset);
	}
	// else --> both mLineNumber and mOffset are -1 --> using empty string
	return str;
}

std::string cfg::Value::getFilenameAndPosition() const
{
	if (mFilename) {
		return *mFilename + getFilePosition();
	}
	return getFilePosition();
}

void cfg::Value::clear()
{
	mLineNumber = -1;
	mOffset = -1;
	mNvpDeep = -1;

	mType = TYPE_NONE;
	mParseBase = 0;
	mParseTextWithQuotes = false;
	mBool = false;
	mFloatingPoint = 0.0;
	mInteger = 0;
	mText.clear();
	mArray.clear();
	mObject.clear();
}

void cfg::Value::setNull()
{
	clear();
	mType = TYPE_NULL;
}

void cfg::Value::setBool(bool value)
{
	clear();
	mType = TYPE_BOOL;
	mParseBase = 2; // two values possible --> false and true --> base is 2
	mBool = value;
	mFloatingPoint = static_cast<float>(value);
	mInteger = static_cast<int>(value);
}

void cfg::Value::setFloatingPoint(float value)
{
	clear();
	mType = TYPE_FLOAT;
	mParseBase = 10;
	mFloatingPoint = value;
#if 0
	mBool = (std::fpclassify(value) != FP_ZERO);
	mInteger = static_cast<int>(value);
#else
	mBool = value >= 0.5f || value <= -0.5f;
	mInteger = static_cast<int>(value + ((value >= 0.0) ? 0.5f : -0.5f));
#endif
}

void cfg::Value::setInteger(int value, unsigned int parseBase)
{
	clear();
	mType = TYPE_INT;
	mParseBase = parseBase;
	mBool = !!value;
	mFloatingPoint = static_cast<float>(value);
	mInteger = value;
}

void cfg::Value::setText(const std::string& text, bool parseTextWithQuotes)
{
	clear();
	mType = TYPE_TEXT;
	mParseTextWithQuotes = parseTextWithQuotes;
	mText = text;
}

void cfg::Value::setComment(const std::string& text)
{
	clear();
	mType = TYPE_COMMENT;
	mText = text;
}

void cfg::Value::setArray()
{
	clear();
	mType = TYPE_ARRAY;
	//mArray.clear(); // already done by clear() member function
}

void cfg::Value::setObject()
{
	clear();
	mType = TYPE_OBJECT;
	//mObject.clear(); // already done by clear() member function
}

bool cfg::Value::equalText(const std::string& text) const
{
	return (mType == TYPE_TEXT && mText == text) ? true : false;
}

bool cfg::Value::isComplexArray() const
{
	if (!isArray()) {
		return false;
	}
	for (const Value& val : mArray) {
		if (!val.isSimple()) {
			// --> find at least one element which is not simple.
			// --> its a complex array
			return true;
		}
	}
	return false;
}

bool cfg::Value::attributeExist(const std::string& attrName,
		bool recursive, bool searchInclObjects, bool searchInclArrays) const
{
	if (searchInclObjects && isObject()) {
		for (const NameValuePair& nvp : mObject) {
			if (nvp.mName.isText() && nvp.mName.mText == attrName) {
				return true;
			}
			if (recursive && nvp.isObject()) {
				if (nvp.mValue.attributeExist(attrName, recursive,
						searchInclObjects, searchInclArrays)) {
					return true;
				}
			}
		}
	}
	if (searchInclArrays && isArray()) {
		for (const Value& val: mArray) {
			if (val.isText() && val.mText == attrName) {
				return true;
			}
			if (recursive && (val.isObject() || val.isArray())) {
				if (val.attributeExist(attrName, recursive,
						searchInclObjects, searchInclArrays)) {
					return true;
				}
			}
		}
	}
	return false;
}

const cfg::NameValuePair* cfg::Value::objectGetValuePair(
		const std::string &attrName) const
{
	std::size_t pairCount = mObject.size();
	std::size_t foundCount = 0;
	const NameValuePair* valuePair = nullptr;

	for (std::size_t i = 0; i < pairCount; i++) {
		if (mObject[i].mName.mType != Value::TYPE_TEXT) {
			continue;
		}
		if (mObject[i].mName.mText != attrName) {
			continue;
		}
		foundCount++;
		valuePair = &mObject[i];
	}
	if (foundCount != 1) {
		return nullptr;
	}
	return valuePair;
}

const cfg::Value* cfg::Value::objectGetValue(const std::string &attrName) const
{
	const NameValuePair* valuePair = objectGetValuePair(attrName);
	if (!valuePair) {
		return nullptr;
	}
	return &valuePair->mValue;
}

bool cfg::Value::objectGetText(const std::string &attrName,
		std::string &attrValue) const
{
	const Value* value = objectGetValue(attrName);
	if (!value) {
		return false;
	}
	if (value->mType != Value::TYPE_TEXT) {
		return false;
	}
	attrValue = value->mText;
	return true;
}

std::string cfg::Value::objectGetText(const std::string &attrName) const
{
	std::string attrValue;
	if (!objectGetText(attrName, attrValue)) {
		return "";
	}
	return attrValue;
}

bool cfg::Value::objectGetInteger(const std::string &attrName, int &attrValue) const
{
	const Value* value = objectGetValue(attrName);
	if (!value) {
		return false;
	}
	if (value->mType != Value::TYPE_INT) {
		return false;
	}
	attrValue = value->mInteger;
	return true;
}

int cfg::Value::objectGetInteger(const std::string &attrName) const
{
	int attrValue = 0;
	if (!objectGetInteger(attrName, attrValue)) {
		return 0;
	}
	return attrValue;
}

bool cfg::Value::objectGetBool(const std::string &attrName, bool &attrValue) const
{
	const Value* value = objectGetValue(attrName);
	if (!value) {
		return false;
	}
	if (value->mType != Value::TYPE_BOOL) {
		return false;
	}
	attrValue = value->mBool;
	return true;
}

bool cfg::Value::objectGetBool(const std::string &attrName) const
{
	bool attrValue = 0;
	if (!objectGetBool(attrName, attrValue)) {
		return false;
	}
	return attrValue;
}

std::vector<const cfg::NameValuePair*> cfg::Value::objectGetValuePairs(
		const std::string &attrName) const
{
	std::vector<const cfg::NameValuePair*> pairs;
	std::size_t pairCount = mObject.size();

	for (std::size_t i = 0; i < pairCount; i++) {
		if (mObject[i].mName.mType != Value::TYPE_TEXT) {
			continue;
		}
		if (mObject[i].mName.mText != attrName) {
			continue;
		}
		pairs.push_back(&mObject[i]);
	}
	return pairs;
}

std::vector<const cfg::Value*> cfg::Value::objectGetValues(
		const std::string &attrName) const
{
	std::vector<const cfg::Value*> values;
	std::size_t pairCount = mObject.size();

	for (std::size_t i = 0; i < pairCount; i++) {
		if (mObject[i].mName.mType != Value::TYPE_TEXT) {
			continue;
		}
		if (mObject[i].mName.mText != attrName) {
			continue;
		}
		values.push_back(&mObject[i].mValue);
	}
	return values;
}

std::vector<const std::string*> cfg::Value::objectGetTexts(
		const std::string &attrName) const
{
	std::vector<const Value*> values = objectGetValues(attrName);
	std::vector<const std::string*> texts;
	texts.reserve(values.size());
	for (std::size_t i = 0; i < values.size(); i++) {
		if (values[i]->mType != Value::TYPE_TEXT) {
			continue;
		}
		texts.push_back(&values[i]->mText);
	}
	return texts;
}

std::vector<int> cfg::Value::objectGetIntegers(const std::string &attrName) const
{
	std::vector<const Value*> values = objectGetValues(attrName);
	std::vector<int> ints;
	ints.reserve(values.size());
	for (std::size_t i = 0; i < values.size(); i++) {
		if (values[i]->mType != Value::TYPE_INT) {
			continue;
		}
		ints.push_back(values[i]->mInteger);
	}
	return ints;
}

int cfg::Value::objectGetAttrIndex(const std::string &attrName) const
{
	std::size_t pairCount = mObject.size();

	for (std::size_t i = 0; i < pairCount; i++) {
		if (mObject[i].mName.mType != Value::TYPE_TEXT) {
			continue;
		}
		if (mObject[i].mName.mText != attrName) {
			continue;
		}
		return static_cast<int>(i);
	}
	return -1;
}

int cfg::Value::objectGet(const SelectRule *rules,
		bool allowRandomSequence, bool allowUnusedValuePairs,
		bool allowEarlyReturn, bool allowDuplicatedNames,
		bool allowDuplicatedRuleNamesWithDiffTypes,
		std::size_t startIndex, std::size_t *outNextIndex,
		EReset reset, std::string* errMsg, std::string* warnings) const
{
	return cfg::objectGet(mObject, rules,
			allowRandomSequence, allowUnusedValuePairs,
			allowEarlyReturn, allowDuplicatedNames,
			allowDuplicatedRuleNamesWithDiffTypes,
			startIndex, outNextIndex,
			reset, errMsg, warnings);
}

cfg::Value cfg::none(int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	Value v;
	v.mLineNumber = lineNumber;
	v.mOffset = offset;
	v.mNvpDeep = nvpDeep;
	v.mFilename = filename;
	v.mType = Value::TYPE_NONE;
	return v;
}

cfg::Value cfg::nullValue(int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	return Value{0, 0, lineNumber, offset, nvpDeep, filename};
}

cfg::Value cfg::boolValue(bool boolValue,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	return Value{boolValue, lineNumber, offset, nvpDeep, filename};
}

cfg::Value cfg::floatValue(float floatingPointValue,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	return Value{floatingPointValue, lineNumber, offset, nvpDeep, filename};
}

cfg::Value cfg::intValue(int integerValue, unsigned int parseBase,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	return Value{integerValue, parseBase, lineNumber, offset, nvpDeep, filename};
}

cfg::Value cfg::text(const std::string& text,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	return Value{text, lineNumber, offset, nvpDeep, filename};
}

cfg::Value cfg::commentValue(const std::string& comment,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	Value v{comment, lineNumber, offset, nvpDeep, filename};
	v.mType = Value::TYPE_COMMENT;
	return v;
}

cfg::Value cfg::array(int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	std::vector<Value> array;
	return Value{array, lineNumber, offset, nvpDeep, filename};
}

cfg::Value cfg::array(const std::vector<Value>& array,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	return Value{array, lineNumber, offset, nvpDeep, filename};
}

cfg::Value cfg::object(int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	std::vector<NameValuePair> object;
	return Value{object, lineNumber, offset, nvpDeep, filename};
}

cfg::Value cfg::object(const std::vector<NameValuePair>& object,
		int lineNumber, int offset, int nvpDeep,
		const std::shared_ptr<const std::string>& filename)
{
	return Value{object, lineNumber, offset, nvpDeep, filename};
}

int cfg::objectGet(const std::vector<NameValuePair>& nvpairsOfObject,
		const SelectRule *rules,
		bool allowRandomSequence, bool allowUnusedValuePairs,
		bool allowEarlyReturn, bool allowDuplicatedNames,
		bool allowDuplicatedRuleNamesWithDiffTypes,
		std::size_t startIndex, std::size_t *outNextIndex,
		EReset reset, std::string* errMsg, std::string* warnings)
{
	std::size_t rulesSize = 0;
	unsigned int finalMustExistCount = 0;
	unsigned int currentMustExistCount = 0;
	if (errMsg) {
		errMsg->clear();
	}
	if (warnings) {
		warnings->clear();
	}
	while (rules[rulesSize].mType != SelectRule::TYPE_UNKNOWN) {
		if (reset != EReset::RESET_NOTHING && rules[rulesSize].mStorePtr.mPtr) {
			switch (rules[rulesSize].mType) {
				case SelectRule::TYPE_UNKNOWN:
					break;
				case SelectRule::TYPE_NULL:
					if (reset == EReset::RESET_EVERYTHING_TO_DEFAULTS) {
						*rules[rulesSize].mStorePtr.mNull = false;
					}
					break;
				case SelectRule::TYPE_BOOL:
					if (reset == EReset::RESET_EVERYTHING_TO_DEFAULTS) {
						*rules[rulesSize].mStorePtr.mBool = false;
					}
					break;
				case SelectRule::TYPE_FLOAT:
					if (reset == EReset::RESET_EVERYTHING_TO_DEFAULTS) {
						*rules[rulesSize].mStorePtr.mFloat = 0.0f;
					}
					break;
				case SelectRule::TYPE_DOUBLE:
					if (reset == EReset::RESET_EVERYTHING_TO_DEFAULTS) {
						*rules[rulesSize].mStorePtr.mDouble = 0.0;
					}
					break;
				case SelectRule::TYPE_INT:
					if (reset == EReset::RESET_EVERYTHING_TO_DEFAULTS) {
						*rules[rulesSize].mStorePtr.mInt = 0;
					}
					break;
				case SelectRule::TYPE_UINT:
					if (reset == EReset::RESET_EVERYTHING_TO_DEFAULTS) {
						*rules[rulesSize].mStorePtr.mUInt = 0;
					}
					break;
				case SelectRule::TYPE_STRING:
					if (reset == EReset::RESET_EVERYTHING_TO_DEFAULTS) {
						rules[rulesSize].mStorePtr.mStr->clear();
					}
					break;
				case SelectRule::TYPE_ARRAY:
					*rules[rulesSize].mStorePtr.mArray = nullptr;
					break;
				case SelectRule::TYPE_OBJECT:
					*rules[rulesSize].mStorePtr.mObject = nullptr;
					break;
				case SelectRule::TYPE_VALUE:
					*rules[rulesSize].mStorePtr.mValue = nullptr;
					break;
				case SelectRule::TYPE_VALUE_PAIR:
					*rules[rulesSize].mStorePtr.mValuePair = nullptr;
					break;
			}
		}
		if (rules[rulesSize].mUsedCount) {
			*rules[rulesSize].mUsedCount = 0;
		}
		if (rules[rulesSize].mRule == SelectRule::RULE_MUST_EXIST) {
			++finalMustExistCount;
		}
		++rulesSize;
	}

	std::size_t pairCount = nvpairsOfObject.size();
	std::size_t curRuleIndex = 0;
	std::size_t storeCount = 0;

	std::size_t usedRuleCounts[64] = {};
	if (rulesSize > 64) {
		if (errMsg) {
			*errMsg = "Too many config rules (" + std::to_string(rulesSize) + " > 64)";
		}
		return -1;
	}

	std::size_t i = startIndex;
	bool prevTypeWasWrong = false;
	for (; storeCount < rulesSize; ++i) {
		if (prevTypeWasWrong) {
			--i; // revert the ++i from loop
		}

		// This check must be made after the prevTypeWasWrong check and
		// not before in the for-condition.
		// It's not possible to use i < pairCount in for() instead because
		// then the logic doesn't work correct if the last name-value pair
		// has a wrong type. Because then ++i would be equal pairCount -->
		// loop finished --> --i not executed --> no next rule for wrong type
		// if last name-value pair has rule with wrong type.
		if (i >= pairCount) {
			break;
		}
		const NameValuePair& vp = nvpairsOfObject[i];

		if (vp.isEmpty() || vp.isComment()) {
			// Should never be possible that prevTypeWasWrong is not already false.
			// But better safe than sorry.
			prevTypeWasWrong = false;
			continue;
		}

		const std::string& vpAttrName =
				(vp.mName.mType == Value::TYPE_TEXT) ?
				vp.mName.mText : "";

		// find the correct rule
		int ri = getRuleIndex(vpAttrName, curRuleIndex,
				rules, rulesSize, allowRandomSequence && !prevTypeWasWrong);
		if (ri < 0) {
			int riWithRandomSequence = getRuleIndex(vpAttrName, curRuleIndex,
					rules, rulesSize, !prevTypeWasWrong);

			// now we can reset prevTypeWasWrong (if it was set). Before its not
			// possible because getRuleIndex() need this variable!
			prevTypeWasWrong = false;

			// no rule found for this attr name
			if (allowUnusedValuePairs) {
				if (riWithRandomSequence >= 0 && warnings) {
					*warnings += "attr name '" + vpAttrName +
							"' has the wrong position/order and is ignored (unused name-value pair).\n";
				}
				continue;
			}
			if (allowEarlyReturn && finalMustExistCount == currentMustExistCount) {
				std::size_t ii = i;
				// here i < pairCount is ok in for() because no --i is possible
				// in this loop. See for() loop above for more infos.
				for (; i < pairCount && storeCount < rulesSize; ++i) {
					const NameValuePair& vp = nvpairsOfObject[i];
					const std::string& vpAttrName =
							(vp.mName.mType == Value::TYPE_TEXT) ?
							vp.mName.mText : "";
					// allowRandomSequence is here independent of prevTypeWasWrong
					// no && !prevTypeWasWrong for getRuleIndex.
					if (getRuleIndex(vpAttrName, curRuleIndex,
							rules, rulesSize, allowRandomSequence) >= 0) {
						if (errMsg) {
							*errMsg = "rule for attr name '" + vpAttrName + "' is not allowed here";
						}
						return -2;
					}
					++curRuleIndex; // no problem if out of range because getRuleIndex() check this
				}
				if (outNextIndex) {
					*outNextIndex = ii;
				}
				return static_cast<int>(storeCount);
			}
			if (errMsg) {
				if (riWithRandomSequence >= 0) {
					*errMsg = "attr name '" + vpAttrName + "' has the wrong position/order. Random sequence is not allowed.";
				}
				else {
					*errMsg = "no rule found for attr name '" + vpAttrName + "'";
				}
			}
			return -3;
		}
		// now we can reset prevTypeWasWrong (if it was set). Before its not
		// possible because getRuleIndex() need this variable!
		prevTypeWasWrong = false;

		curRuleIndex = ri;
		++usedRuleCounts[curRuleIndex];
		if (rules[curRuleIndex].mUsedCount) {
			++*rules[curRuleIndex].mUsedCount;
		}
		// check if it was the first time that the rule was used and if the rule must be used
		if (usedRuleCounts[curRuleIndex] == 1 &&
				rules[curRuleIndex].mRule == SelectRule::RULE_MUST_EXIST) {
			++currentMustExistCount;
		}

		const SelectRule& rule = rules[curRuleIndex];
		static unsigned int lookup[] = {
			SelectRule::ALLOW_NONE,   // for Value::TYPE_NONE   = 0
			SelectRule::ALLOW_NULL,   // for Value::TYPE_NULL   = 1
			SelectRule::ALLOW_BOOL,   // for Value::TYPE_BOOL   = 2
			SelectRule::ALLOW_FLOAT,  // for Value::TYPE_FLOAT  = 3
			SelectRule::ALLOW_INT,    // for Value::TYPE_INT    = 4
			SelectRule::ALLOW_TEXT,   // for Value::TYPE_TEXT   = 5
			0, // ALLOW_COMMENT not exist // for Value::TYPE_COMMENT = 6
			SelectRule::ALLOW_ARRAY,  // for Value::TYPE_ARRAY  = 7
			SelectRule::ALLOW_OBJECT, // for Value::TYPE_OBJECT = 8
		};
		if (!(rule.mAllowedTypes & lookup[vp.mValue.mType])) {
			if (allowDuplicatedRuleNamesWithDiffTypes) {
				prevTypeWasWrong = true;
				// revert rule count because this rule was not used (because of wrong type)

				// check if it was the first time that the rule was used and if the rule must be used
				// then the "current must exist count" must also be reverted
				if (usedRuleCounts[curRuleIndex] == 1 &&
						rules[curRuleIndex].mRule == SelectRule::RULE_MUST_EXIST) {
					--currentMustExistCount;
				}
				--usedRuleCounts[curRuleIndex];
				if (rules[curRuleIndex].mUsedCount) {
					--*rules[curRuleIndex].mUsedCount;
				}
				curRuleIndex++; // no problem if out of range because getRuleIndex() check this
				continue;
			}
			if (errMsg) {
				*errMsg = "Type is not allowed for rule '" + vpAttrName +
						"'. allowed " + std::to_string(rule.mAllowedTypes) +
						", lookup " + std::to_string(lookup[vp.mValue.mType]) +
						", type " + std::to_string(vp.mValue.mType);
			}
			return -5;
		}

		// The duplication check must be here after the type check. Because
		// if the type is wrong then the rule is not applied and therefore
		// not duplicated.
		if (usedRuleCounts[curRuleIndex] > 1) {
			if (allowDuplicatedNames) {
				curRuleIndex++; // no problem if out of range because getRuleIndex() check this
				continue; // ignore duplicates --> jump to next
			}
			if (errMsg) {
				*errMsg = "found duplicated rule for attr name '" + vpAttrName + "'";
			}
			return -4;
		}

		switch (rule.mType) {
			case SelectRule::TYPE_UNKNOWN:
				return -1; // unknown is not allowed
			case SelectRule::TYPE_NULL:
				*rule.mStorePtr.mNull = true;
				break;
			case SelectRule::TYPE_BOOL:
				*rule.mStorePtr.mBool = vp.mValue.mBool;
				break;
			case SelectRule::TYPE_FLOAT:
				*rule.mStorePtr.mFloat = vp.mValue.mFloatingPoint;
				break;
			case SelectRule::TYPE_DOUBLE:
				*rule.mStorePtr.mDouble = vp.mValue.mFloatingPoint;
				break;
			case SelectRule::TYPE_INT:
				*rule.mStorePtr.mInt = vp.mValue.mInteger;
				break;
			case SelectRule::TYPE_UINT:
				*rule.mStorePtr.mUInt = (vp.mValue.mInteger >= 0) ?
						vp.mValue.mInteger : 0;
				break;
			case SelectRule::TYPE_STRING:
				*rule.mStorePtr.mStr = vp.mValue.mText;
				break;
			case SelectRule::TYPE_ARRAY:
				*rule.mStorePtr.mArray = &vp.mValue.mArray;
				break;
			case SelectRule::TYPE_OBJECT:
				*rule.mStorePtr.mObject = &vp.mValue.mObject;
				break;
			case SelectRule::TYPE_VALUE:
				*rule.mStorePtr.mValue = &vp.mValue;
				break;
			case SelectRule::TYPE_VALUE_PAIR:
				*rule.mStorePtr.mValuePair = &vp;
				break;
		}
		curRuleIndex++; // no problem if out of range because getRuleIndex() check this
		storeCount++;
	}

	// now check if the rules are complied
#if 1
	for (std::size_t ri = 0; ri < rulesSize; ri++) {
		if (rules[ri].mRule == SelectRule::RULE_MUST_EXIST && !usedRuleCounts[ri]) {
			if (errMsg) {
				*errMsg = "rule '" + rules[ri].mName + "' with index " +
						std::to_string(ri) + " is not used (but is RULE_MUST_EXIST)";
			}
			return -6;
		}
	}
#endif
	// this version should be enougth. Loop above is not necessary.
	if (currentMustExistCount < finalMustExistCount) {
		if (errMsg) {
			*errMsg = "wrong used count for 'must exist' rules. " +
					std::to_string(currentMustExistCount) + " < " +
					std::to_string(finalMustExistCount);
		}
		return -7;
	}
	if (outNextIndex) {
		*outNextIndex = i;
	}
	return static_cast<int>(storeCount);
}

//-----------------------------------------------

cfg::NameValuePair::NameValuePair()
	:mName(), mValue(), mDeep(-1)
{
}

cfg::NameValuePair::NameValuePair(const Value& name, const Value& value, int deep)
		:mName(name), mValue(value), mDeep(deep)
{
}

cfg::NameValuePair::NameValuePair(NameValuePair&& other)
	:mName(std::move(other.mName)),
	mValue(std::move(other.mValue)),
	mDeep(std::move(other.mDeep))
{
	other.mDeep = -1;
}

cfg::NameValuePair& cfg::NameValuePair::operator=(NameValuePair&& other)
{
	if (this == &other) {
		return *this;
	}

	mName = std::move(other.mName);
	mValue = std::move(other.mValue);
	mDeep = std::move(other.mDeep);

	other.mDeep = -1;

	return *this;
}

void cfg::NameValuePair::clear()
{
	mName.clear();
	mValue.clear();
}

void cfg::NameValuePair::setTextBool(const std::string& attrName, bool attrValue)
{
	mName.setText(attrName);
	mValue.setBool(attrValue);
}

void cfg::NameValuePair::setTextFloat(const std::string& attrName, float attrValue)
{
	mName.setText(attrName);
	mValue.setFloatingPoint(attrValue);
}

void cfg::NameValuePair::setTextInt(const std::string& attrName, int attrValue)
{
	mName.setText(attrName);
	mValue.setInteger(attrValue);
}

void cfg::NameValuePair::setTextText(const std::string& attrName, const std::string& attrValue)
{
	mName.setText(attrName);
	mValue.setText(attrValue);
}

void cfg::NameValuePair::setTextArray(const std::string& attrName)
{
	mName.setText(attrName);
	mValue.setArray();

}

void cfg::NameValuePair::setComment(const std::string& text)
{
	mName.setComment(text);
	mValue.clear();
}

void cfg::NameValuePair::setObject()
{
	mName.clear();
	mValue.setObject();
}

void cfg::NameValuePair::setObject(const std::string &objectName)
{
	mName.setText(objectName);
	mValue.setObject();
}

cfg::NameValuePair cfg::empty(int deep)
{
	return NameValuePair(none(), none(), deep);
}

cfg::NameValuePair cfg::comment(const std::string& comment, int deep)
{
	return pair(commentValue(comment), none(), deep);
}

cfg::NameValuePair cfg::single(const Value& name, int deep)
{
	return NameValuePair(name, none(), deep);
}

cfg::NameValuePair cfg::pair(const Value& name, const Value& value, int deep)
{
	return NameValuePair(name, value, deep);
}


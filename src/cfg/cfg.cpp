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
				return curRuleIndex;
			}
			// first search after the current rule index is also allowed
			// if random sequence is false because the sequence is unchanged.
			// Only some rules can be skipped.
			// If a rule should not be skipped then SelectRule::RULE_MUST_EXIST
			// should be used.
			for (std::size_t i = curRuleIndex + 1; i < rulesSize; i++) {
				if (ruleName == rules[i].mName) {
					return i;
				}
			}
			if (!allowRandomSequence) {
				return -1; // no other rule allowed
			}
			// not found --> search before the current rule index
			for (std::size_t i = 0; i < curRuleIndex; i++) {
				if (ruleName == rules[i].mName) {
					return i;
				}
			}
			return -1; // no rule found
		}
	}
}

cfg::Value::Value()
		:mLineNumber(-1), mOffset(-1),
		mType(TYPE_NONE),
		mParseBase(0),
		mBool(false),
		mFloatingPoint(0.0f),
		mInteger(0),
		mText(""),
		mObject()
{
}

cfg::Value::~Value()
{
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

	mType = TYPE_NONE;
	mParseBase = 0;
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
	mParseBase = 1;
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

void cfg::Value::setText(const std::string& text)
{
	clear();
	mType = TYPE_TEXT;
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

const cfg::NameValuePair* cfg::Value::objectGetValuePair(
		const std::string &attrName) const
{
	unsigned int pairCount = mObject.size();
	unsigned int foundCount = 0;
	const NameValuePair* valuePair = nullptr;

	for (unsigned int i = 0; i < pairCount; i++) {
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
		return NULL;
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
	unsigned int pairCount = mObject.size();

	for (unsigned int i = 0; i < pairCount; i++) {
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
	unsigned int pairCount = mObject.size();

	for (unsigned int i = 0; i < pairCount; i++) {
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
	unsigned int pairCount = mObject.size();

	for (unsigned int i = 0; i < pairCount; i++) {
		if (mObject[i].mName.mType != Value::TYPE_TEXT) {
			continue;
		}
		if (mObject[i].mName.mText != attrName) {
			continue;
		}
		return i;
	}
	return -1;
}

int cfg::Value::objectGet(const SelectRule *rules,
		bool allowRandomSequence,
		bool allowUnusedValuePairs, bool allowEarlyReturn,
		bool allowDuplicatedNames,
		bool allowDuplicatedRuleNamesWithDiffTypes,
		std::size_t startIndex, std::size_t *outNextIndex,
		EReset reset,
		std::string* errMsg) const
{
	std::size_t rulesSize = 0;
	unsigned int finalMustExistCount = 0;
	unsigned int currentMustExistCount = 0;
	if (errMsg) {
		errMsg->clear();
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

	std::size_t pairCount = mObject.size();
	std::size_t curRuleIndex = 0;
	std::size_t storeCount = 0;

	std::size_t usedRuleCounts[64];
	memset(usedRuleCounts, 0, sizeof(usedRuleCounts));
	if (rulesSize > 64) {
		if (errMsg) {
			*errMsg = "Too many config rules (" + std::to_string(rulesSize) + " > 64)";
		}
		return -1;
	}

	std::size_t i = startIndex;
	bool prevTypeWasWrong = false;
	for (; i < pairCount && storeCount < rulesSize; ++i) {
		if (prevTypeWasWrong) {
			--i; // revert the ++i from loop
		}
		const NameValuePair& vp = mObject[i];

		if (vp.isEmpty() || vp.isComment()) {
			continue;
		}

		const std::string& vpAttrName =
				(vp.mName.mType == Value::TYPE_TEXT) ?
				vp.mName.mText : "";

		// find the correct rule
		int ri = getRuleIndex(vpAttrName, curRuleIndex,
				rules, rulesSize, allowRandomSequence && !prevTypeWasWrong);
		// now we can reset prevTypeWasWrong (if it was set). Before its not
		// possible because getRuleIndex() need this variable!
		prevTypeWasWrong = false;
		if (ri < 0) {
			// no rule found for this attr name
			if (allowUnusedValuePairs) {
				continue;
			}
			if (allowEarlyReturn && finalMustExistCount == currentMustExistCount) {
				std::size_t ii = i;
				for (; i < pairCount && storeCount < rulesSize; ++i) {
					const NameValuePair& vp = mObject[i];
					const std::string& vpAttrName =
							(vp.mName.mType == Value::TYPE_TEXT) ?
							vp.mName.mText : "";
					if (getRuleIndex(vpAttrName, curRuleIndex,
							rules, rulesSize, allowRandomSequence && !prevTypeWasWrong) >= 0) {
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
				return storeCount;
			}
			if (errMsg) {
				*errMsg = "no rule found for attr name '" + vpAttrName + "'";
			}
			return -3;
		}
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
	return storeCount;
}

//-----------------------------------------------

cfg::NameValuePair::NameValuePair()
	:mDeep(-1)
{
}

cfg::NameValuePair::~NameValuePair()
{
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


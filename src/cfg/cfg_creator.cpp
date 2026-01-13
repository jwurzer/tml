#include <cfg/cfg_creator.h>
#include <sstream>

namespace
{
	cfg::Value& getCurrent(cfg::Value& cfg, unsigned int deep, unsigned int arrayIndex)
	{
		if (deep == 0) {
			return cfg;
		}
		// --> deep > 1
		if (cfg.isObject()) {
			if (cfg.mObject.empty()) {
				cfg.mObject.emplace_back();

				cfg.mObject.back().mName.setNull();
				cfg.mObject.back().mValue.setObject();
			}
			return getCurrent(cfg.mObject.back().mValue, deep - 1, arrayIndex);
		}
		if (cfg.isArray()) {
			// --> deep > 1
			if (cfg.mArray.empty()) {
				cfg.mArray.emplace_back();
				cfg.mArray.back().setObject();
			}
			return getCurrent(cfg.mArray.back(), deep - 1, arrayIndex);
		}
		return cfg;
	}
}

cfg::CfgCreator::CfgCreator()
{
	mCfg.setObject();
}

cfg::CfgCreator& cfg::CfgCreator::configParseTextWithQuotes(bool enable)
{
	isParseTextWithQuotesForName = enable;
	isParseTextWithQuotesForValue = enable;
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::configParseTextWithQuotesForName(bool enable)
{
	isParseTextWithQuotesForName = enable;
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::configParseTextWithQuotesForValue(bool enable)
{
	isParseTextWithQuotesForValue = enable;
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::empty()
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::comment(const std::string& comment,
		bool withSpace)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
		if (withSpace) {
			val.mObject.back().setComment(" " + comment);
		}
		else {
			val.mObject.back().setComment(comment);
		}
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		if (withSpace) {
			val.mArray.back().setComment(" " + comment);
		}
		else {
			val.mArray.back().setComment(comment);
		}
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::pushObject(const std::string& name)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
		val.mObject.back().setObject(name);
		val.mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
		++mDeep;
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		if (!name.empty()) {
			addWarning("pushObject: object name '" + name + "' is ignored inside an array.");
		}
		val.mArray.back().setObject();
		++mDeep;
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::popObject()
{
	Value& val = getCurrent();
	if (val.isObject()) {
		if (mDeep > 0) {
			--mDeep;
		}
		else {
			addWarning("popObject: deep is already 0.");
		}
	}
	else {
		addWarning("popObject: is no object.");
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::pushArray(const std::string& name)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
		val.mObject.back().setTextArray(name);
		val.mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
		++mDeep;
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		if (!name.empty()) {
			addWarning("pushArray: array name '" + name + "' is ignored inside an array.");
		}
		val.mArray.back().setArray();
		++mDeep;
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::popArray()
{
	Value& val = getCurrent();
	if (val.isArray()) {
		if (mDeep > 0) {
			--mDeep;
		}
		else {
			addWarning("popArray: deep is already 0.");
		}
	}
	else {
		addWarning("popArray: is no array.");
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::nvpNull(const std::string& name)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
		val.mObject.back().mName.setText(name, isParseTextWithQuotesForName);
		val.mObject.back().mValue.setNull();
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setObject();
		val.mArray.back().mObject.emplace_back();
		val.mArray.back().mObject.back().mName.setText(name, isParseTextWithQuotesForName);
		val.mArray.back().mObject.back().mName.setNull();
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::nvpBool(const std::string& name, bool value)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
		val.mObject.back().setTextBool(name, value);
		val.mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setObject();
		val.mArray.back().mObject.emplace_back();
		val.mArray.back().mObject.back().setTextBool(name, value);
		val.mArray.back().mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::nvpFloat(const std::string& name, float value)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
		val.mObject.back().setTextFloat(name, value);
		val.mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setObject();
		val.mArray.back().mObject.emplace_back();
		val.mArray.back().mObject.back().setTextFloat(name, value);
		val.mArray.back().mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::nvpInt(const std::string& name, int value)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
		val.mObject.back().setTextInt(name, value);
		val.mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setObject();
		val.mArray.back().mObject.emplace_back();
		val.mArray.back().mObject.back().setTextInt(name, value);
		val.mArray.back().mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::nvpText(const std::string& name, const std::string& value)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		val.mObject.emplace_back();
		val.mObject.back().setTextText(name, value);
		val.mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
		val.mObject.back().mValue.mParseTextWithQuotes = isParseTextWithQuotesForValue;
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setObject();
		val.mArray.back().mObject.emplace_back();
		val.mArray.back().mObject.back().setTextText(name, value);
		val.mArray.back().mObject.back().mName.mParseTextWithQuotes = isParseTextWithQuotesForName;
		val.mArray.back().mObject.back().mValue.mParseTextWithQuotes = isParseTextWithQuotesForValue;
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::valNull()
{
	Value& val = getCurrent();
	if (val.isObject()) {
		if (!mAssign) {
			val.mObject.emplace_back();
			val.mObject.back().mName.setNull();
		}
		else {
			if (val.mObject.empty()) {
				val.mObject.emplace_back();
				val.mObject.back().mName.setNull(); // set unused name to null
				addWarning("valNull: assign is used at an invalid status.");
			}
			val.mObject.back().mValue.setNull();
			mAssign = false;
		}
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setNull();
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::valBool(bool value)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		if (!mAssign) {
			val.mObject.emplace_back();
			val.mObject.back().mName.setBool(value);
		}
		else {
			if (val.mObject.empty()) {
				val.mObject.emplace_back();
				val.mObject.back().mName.setNull(); // set unused name to null
				addWarning("valBool: assign is used at an invalid status.");
			}
			val.mObject.back().mValue.setBool(value);
			mAssign = false;
		}
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setBool(value);
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::valFloat(float value)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		if (!mAssign) {
			val.mObject.emplace_back();
			val.mObject.back().mName.setFloatingPoint(value);
		}
		else {
			if (val.mObject.empty()) {
				val.mObject.emplace_back();
				val.mObject.back().mName.setNull(); // set unused name to null
				addWarning("valFloat: assign is used at an invalid status.");
			}
			val.mObject.back().mValue.setFloatingPoint(value);
			mAssign = false;
		}
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setFloatingPoint(value);
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::valInt(int value)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		if (!mAssign) {
			val.mObject.emplace_back();
			val.mObject.back().mName.setInteger(value);
		}
		else {
			if (val.mObject.empty()) {
				val.mObject.emplace_back();
				val.mObject.back().mName.setNull(); // set unused name to null
				addWarning("valInt: assign is used at an invalid status.");
			}
			val.mObject.back().mValue.setInteger(value);
			mAssign = false;
		}
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setInteger(value);
	}
	checkForResetAssign();
	return *this;
}

cfg::CfgCreator& cfg::CfgCreator::valText(const std::string& value)
{
	Value& val = getCurrent();
	if (val.isObject()) {
		if (!mAssign) {
			val.mObject.emplace_back();
			val.mObject.back().mName.setText(value, isParseTextWithQuotesForName);
		}
		else {
			if (val.mObject.empty()) {
				val.mObject.emplace_back();
				val.mObject.back().mName.setNull(); // set unused name to null
				addWarning("valText: assign is used at an invalid status.");
			}
			val.mObject.back().mValue.setText(value, isParseTextWithQuotesForValue);
			mAssign = false;
		}
	}
	else if (val.isArray()) {
		val.mArray.emplace_back();
		val.mArray.back().setText(value);
		// Currently an array is only supported for a value by CfgCreator.
		// Using an array for the name is currently NOT possible by the CfgCreator.
		// --> use value
		val.mArray.back().mParseTextWithQuotes = isParseTextWithQuotesForValue;
	}
	checkForResetAssign();
	return *this;
}

// this functions does NOT use checkForResetAssign(). makes no sense here!
cfg::CfgCreator& cfg::CfgCreator::assign()
{
	if (mAssign) {
		addWarning("assign: is already set");
		return *this;
	}
	Value& val = getCurrent();
	if (!val.isObject()) {
		addWarning("assign: No object for assign.");
		return *this;
	}
	if (val.mObject.empty()) {
		addWarning("assign: Empty object not allowed.");
		return *this;
	}
	const NameValuePair& nvp = val.mObject.back();
	if (nvp.isEmptyOrComment() || !nvp.mValue.isEmpty()) {
		addWarning("assign: Not allowed for this name value pair.");
		return *this;
	}
	mAssign = true;
	return *this;
}

cfg::Value cfg::CfgCreator::getCfg() const
{
	return mCfg;
}

std::string cfg::CfgCreator::getWarningsAsString() const
{
	std::stringstream ss;
	for (const std::string warning : mWarnings) {
		ss << warning;
		ss << "\n";
	}
	return ss.str();
}

cfg::Value& cfg::CfgCreator::getCurrent()
{
	return ::getCurrent(mCfg, mDeep, 0);
}

void cfg::CfgCreator::addWarning(const std::string& warning)
{
	mWarnings.push_back(warning);
}

void cfg::CfgCreator::checkForResetAssign()
{
	if (mAssign) {
		addWarning("assign: is not used --> reset assign.");
		mAssign = false;
	}
}

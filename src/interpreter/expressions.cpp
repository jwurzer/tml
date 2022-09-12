#include <interpreter/expressions.h>
#include <cfg/cfg_enum_string.h>
#include <tml/tml_string.h>
#include <cmath>

namespace cfg
{
	namespace expressions
	{
		namespace
		{
			std::string getValueAsString(const cfg::Value& value)
			{
				std::string str = cfg::tmlstring::valueToString(0, value);
				if (!str.empty() && str[str.size() - 1] == '\n') {
					str.pop_back();
				}
				return str;
			}

			std::string convertToText(const cfg::Value& value) {
				if (value.isText()) {
					return value.mText;
				}
				return getValueAsString(value);
			}

			bool isParameterCountCorrect(const std::string& funcName,
					const std::vector<cfg::Value>& args,
					unsigned int expectedParamCount, std::ostream& errMsg)
			{
				if (args.size() != expectedParamCount) {
					errMsg << funcName << "() can't be called with " << args.size() <<
							(args.size() == 1 ? " parameter." : " parameters.") <<
							" Only " << expectedParamCount <<
							(expectedParamCount == 1 ? " parameter" : " parameters") <<
							" is allowed." << std::endl;
					return false;
				}
				return true;
			}

			bool areParameterNumbers(const std::string& funcName,
					const std::vector<cfg::Value>& args,
					std::ostream& errMsg)
			{
				unsigned int paramNr = 1;
				for (const cfg::Value& arg : args) {
					if (!arg.isNumber()) {
						errMsg << funcName << "(): parameter " << paramNr << " must be a number" << std::endl;
						return false;
					}
					++paramNr;
				}
				return true;
			}

			bool functionAbs(Context& /*context*/, const std::string& funcName,
					const std::vector<cfg::Value>& args, cfg::Value& result,
					std::ostream& errMsg)
			{
				if (!isParameterCountCorrect(funcName, args, 1, errMsg)) {
					return false;
				}
				if (!areParameterNumbers(funcName, args, errMsg)) {
					return false;
				}
				const cfg::Value& arg = args[0];
				if (arg.isInteger()) {
					result.setInteger(arg.mInteger < 0 ? -arg.mInteger : arg.mInteger, arg.mParseBase);
				}
				else {
					// --> can only be a float
					result.setFloatingPoint(arg.mFloatingPoint < 0.0f ? -arg.mFloatingPoint : arg.mFloatingPoint);
				}
				return true;
			}

			bool functionBool(Context& /*context*/, const std::string& funcName,
					const std::vector<cfg::Value>& args, cfg::Value& result,
					std::ostream& errMsg)
			{
				if (!isParameterCountCorrect(funcName, args, 1, errMsg)) {
					return false;
				}
				const cfg::Value& arg = args[0];
				if (arg.isBool()) {
					result = arg;
				}
				else if (arg.isInteger()) {
					result.setBool(static_cast<bool>(arg.mInteger));
				}
				else if (arg.isFloat()) {
					result.setBool(std::fpclassify(arg.mFloatingPoint) != FP_ZERO);
					//result.setBool(static_cast<bool>(arg.mFloatingPoint));
				}
				else if (arg.isText()) {
					result.setBool(arg.mText == "true");
				}
				else {
					result.setBool(false);
				}
				return true;
			}

			bool functionInt(Context& /*context*/, const std::string& funcName,
					const std::vector<cfg::Value>& args, cfg::Value& result,
					std::ostream& errMsg)
			{
				if (!isParameterCountCorrect(funcName, args, 1, errMsg)) {
					return false;
				}
				const cfg::Value& arg = args[0];
				if (arg.isBool()) {
					result.setInteger(static_cast<int>(arg.mBool), 2);
				}
				else if (arg.isInteger()) {
					result = arg;
				}
				else if (arg.isFloat()) {
					result.setInteger(static_cast<int>(arg.mFloatingPoint), 10);
				}
				else if (arg.isText()) {
					errMsg << "TODO: impl text to int for int()" << std::endl;
					return false;
				}
				else {
					result.setInteger(0, 10);
				}
				return true;
			}

			bool functionFloat(Context& /*context*/, const std::string& funcName,
					const std::vector<cfg::Value>& args, cfg::Value& /*result*/,
					std::ostream& errMsg)
			{
				if (!isParameterCountCorrect(funcName, args, 1, errMsg)) {
					return false;
				}
				//const cfg::Value& arg = args[0];
				errMsg << "TODO: impl float()" << std::endl;
				return false;
			}
		}
	}
}

bool cfg::expressions::CallExpression::interpret(Context& context,
		cfg::Value& result, std::ostream& errMsg) const
{
	cfg::Value funcNameResult;
	if (!mFunction->interpret(context, funcNameResult, errMsg)) {
		return false;
	}
	if (!funcNameResult.isText()) {
		errMsg << "result for functionname must be a text" << std::endl;
		return false;
	}
	if (funcNameResult.mParseTextWithQuotes) {
		errMsg << "functionname must be without quotes" << std::endl;
		return false;
	}

	const std::string& funcName = funcNameResult.mText;

	std::vector<cfg::Value> argsResults;
	argsResults.resize(mArgs.size());
	unsigned int i = 0;
	for (const std::unique_ptr<Expression>& arg : mArgs) {
		if (!arg->interpret(context, argsResults[i], errMsg)) {
			errMsg << "Can't interpret parameter " << (i + 1) <<
					" for functioncall '" << funcName << "'" << std::endl;
		}
		++i;
	}

	if (funcName == "abs") {
		return functionAbs(context, funcName, argsResults, result, errMsg);
	}
	if (funcName == "bool") {
		return functionBool(context, funcName, argsResults, result, errMsg);
	}
	if (funcName == "int") {
		return functionInt(context, funcName, argsResults, result, errMsg);
	}
	if (funcName == "float") {
		return functionFloat(context, funcName, argsResults, result, errMsg);
	}
	if (funcName == "str") {
		errMsg << "TODO: impl str()" << std::endl;
		return false;
	}

	errMsg << "Can't find function '" << funcName << "'" << std::endl;
	return false;
}

void cfg::expressions::ValueExpression::print(std::ostream& builder) const
{
	builder << getValueAsString(mValue);
}

void cfg::expressions::ValueExpression::printEx(std::ostream& builder) const
{
	builder << "(" << getValueAsString(mValue) << ")";
}

bool cfg::expressions::OperatorExpression::interpret(Context& context,
		cfg::Value& result, std::ostream& errMsg) const
{
	cfg::Value leftResult;
	if (!mLeft->interpret(context, leftResult, errMsg)) {
		return false;
	}
	cfg::Value rightResult;
	if (!mRight->interpret(context, rightResult, errMsg)) {
		return false;
	}
	switch (mOperator) {
		case TokenType::PLUS:     //  +
			if (leftResult.isText() || rightResult.isText()) {
				result.setText(convertToText(leftResult) + convertToText(rightResult));
				return true;
			}
			if (leftResult.isNumber() && rightResult.isNumber()) {
				if (leftResult.isInteger() && rightResult.isInteger()) {
					unsigned int parseBase =
							(leftResult.mParseBase == rightResult.mParseBase) ?
									leftResult.mParseBase : 10;
					result.setInteger(leftResult.mInteger + rightResult.mInteger, parseBase);
					return true;
				}
				// --> calc as floats
				result.setFloatingPoint(leftResult.mFloatingPoint + rightResult.mFloatingPoint);
				return true;
			}
			break;
		case TokenType::MINUS:    //  -
			if (leftResult.isNumber() && rightResult.isNumber()) {
				if (leftResult.isInteger() && rightResult.isInteger()) {
					unsigned int parseBase =
							(leftResult.mParseBase == rightResult.mParseBase) ?
									leftResult.mParseBase : 10;
					result.setInteger(leftResult.mInteger - rightResult.mInteger, parseBase);
					return true;
				}
				// --> calc as floats
				result.setFloatingPoint(leftResult.mFloatingPoint - rightResult.mFloatingPoint);
				return true;
			}
			break;
		case TokenType::ASTERISK: //  *
			if (leftResult.isNumber() && rightResult.isNumber()) {
				if (leftResult.isInteger() && rightResult.isInteger()) {
					unsigned int parseBase =
							(leftResult.mParseBase == rightResult.mParseBase) ?
									leftResult.mParseBase : 10;
					result.setInteger(leftResult.mInteger * rightResult.mInteger, parseBase);
					return true;
				}
				// --> calc as floats
				result.setFloatingPoint(leftResult.mFloatingPoint * rightResult.mFloatingPoint);
				return true;
			}
			break;
		case TokenType::SLASH:    //  /
			if (leftResult.isNumber() && rightResult.isNumber()) {
				if (leftResult.isInteger() && rightResult.isInteger()) {
					unsigned int parseBase =
							(leftResult.mParseBase == rightResult.mParseBase) ?
									leftResult.mParseBase : 10;
					result.setInteger(leftResult.mInteger / rightResult.mInteger, parseBase);
					return true;
				}
				// --> calc as floats
				result.setFloatingPoint(leftResult.mFloatingPoint / rightResult.mFloatingPoint);
				return true;
			}
			break;
		default: {
			char ch = tokentype::punctuator(mOperator);
			errMsg << (ch ? std::string(1, ch) : tokentype::toString(mOperator)) <<
				" is not supported as binary arithmetic expression.";
			return false;
		}
	}

	// --> a supported operator is used but this operator doesn't support these value types.
	errMsg << enumstring::getValueTypeAsString(leftResult.mType) << " " <<
			tokentype::punctuator(mOperator) << " " <<
			enumstring::getValueTypeAsString(rightResult.mType) <<
			" is not supported for these types as binary arithmetic expression.";
	return false;
}

bool cfg::expressions::PrefixExpression::interpret(Context& context,
		cfg::Value& result, std::ostream& errMsg) const
{
	cfg::Value rightResult;
	if (!mRight->interpret(context, rightResult, errMsg)) {
		return false;
	}
	switch (mOperator) {
		case TokenType::PLUS:     //  +
			if (rightResult.isNumber()) {
				// no changes necessary. take directly rightResult as result.
				result = rightResult;
				return true;
			}
			break;
		case TokenType::MINUS:    //  -
			if (rightResult.isNumber()) {
				if (rightResult.isInteger()) {
					result.setInteger(-rightResult.mInteger, rightResult.mParseBase);
					return true;
				}
				// --> calc as floats
				result.setFloatingPoint(-rightResult.mFloatingPoint);
				return true;
			}
			break;
		default:
			char ch = tokentype::punctuator(mOperator);
			errMsg << (ch ? std::string(1, ch) : tokentype::toString(mOperator)) <<
					" is not supported as prefix unary arithmetic expression.";
			return false;
	}
	// --> a supported operator is used but this operator doesn't support this value type.
	errMsg << tokentype::punctuator(mOperator) << " " <<
			enumstring::getValueTypeAsString(rightResult.mType) <<
			" is not supported for this type as prefix unary arithmetic expression.";
	return false;
}

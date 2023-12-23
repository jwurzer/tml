#include <interpreter/interpreter.h>
#include <interpreter/cfg_parser.h>
#include <interpreter/cfg_lexer.h>
//#include <cfg/cfg_string.h>

int cfg::interpreter::interpretAndReplaceExprValue(cfg::Value& exprResultValue,
		bool allowInterpretationWithQuotes, std::ostream& errMsg)
{
	if (!exprResultValue.isArray()) {
		// nothing to do
		return 0;
	}
	unsigned int count = static_cast<unsigned int>(exprResultValue.mArray.size());
	std::unique_ptr<cfg::CfgParser> parser; // only created at first usage
	cfg::Value fullResult;
	fullResult.setArray();
	unsigned int expressionCount = 0;
	// nextStartIndex ... next start for an expression
	unsigned int nextStartIndex = 0;
	for (unsigned int i = 1; i < count; ++i) {
		if (i <= nextStartIndex) {
			continue;
		}
		// --> i > nextStartIndex --> i can only be >= 1
		const cfg::Value& val = exprResultValue.mArray[i];
		if (!val.isText() || val.mText.length() != 1 || val.mText[0] != '(' ||
				(!allowInterpretationWithQuotes && val.mParseTextWithQuotes)) {
			continue;
		}
		const cfg::Value& prev = exprResultValue.mArray[i - 1];
		if (!prev.isText() || prev.mText.length() < 2 || prev.mText[0] != '_') {
			continue;
		}
		if (prev.mText != "_i" && prev.mText != "_ii" &&
				prev.mText != "_fi" && prev.mText != "_ti") {
			continue;
		}
		if (!allowInterpretationWithQuotes && prev.mParseTextWithQuotes) {
			continue;
		}
		// --> expression for interpreter start with
		// '_i (',  '_ii (',  '_fi ('  or  '_ti ('
		++expressionCount;
		if (!parser) {
			std::unique_ptr<cfg::CfgLexer> lexer(new cfg::CfgLexer(exprResultValue, allowInterpretationWithQuotes));
			parser = std::unique_ptr<cfg::CfgParser>(new cfg::CfgParser(std::move(lexer)));
		}
		unsigned int curStartIndex = i - 1;

		// ci ... copy index
		for (unsigned int ci = nextStartIndex; ci < curStartIndex; ++ci) {
			// without std::move() because we doesn't want change the original input.
			// Because if an error happened exprResultValue should be unchanged.
			//std::cout << "copy " << ci << std::endl;
			fullResult.mArray.push_back(exprResultValue.mArray[ci]);
		}

		int parenCount = 1;
		for (++i; i < count && parenCount > 0; ++i) {
			const cfg::Value& next = exprResultValue.mArray[i];
			if (next.isText() && !next.mParseTextWithQuotes && next.mText.size() == 1) {
				char ch = next.mText[0];
				if (ch == '(') {
					++parenCount;
				}
				else if (ch == ')') {
					--parenCount;
				}
			}
		}
		if (parenCount > 0) {
			errMsg << "Can't find ending" << std::endl;
			return -1;
		}
		if (!parser->getTokenIterator().setRangePosition(curStartIndex, i)) {
			return -1;
		}
		parser->reset();
		nextStartIndex = i;
		//std::cout << "interpreter start with '" << prev.mText << " " << val.mText << "'" << std::endl;
		//std::cout << "interpreter from index '" << curStartIndex << "' to '" << (i - 1) << "'" << std::endl;

		// other loop make ++i --> --i would "normally" be necessary here.
		// But not necessary because start sequence for interpreter takes
		// to token like '_i (',  '_ii (',  '_fi ('  or  '_ti ('
		// not necessary: --i;

		unsigned int errorCount = 0;
		std::unique_ptr<expressions::Expression> expr = parser->parseFullExpression(errorCount);
		if (errorCount) {
			errMsg << "error count " << errorCount << std::endl;
			return -1;
		}
		expressions::Context context(allowInterpretationWithQuotes);
		cfg::Value exprResult;
		if (!expr->interpret(context, exprResult, errMsg)) {
			return -1;
		}
		fullResult.mArray.push_back(std::move(exprResult));

		unsigned int newTokenPosition = parser->getTokenIterator().getPosition();
		if (newTokenPosition != i) {
			errMsg << "wrong internal state" << std::endl;
			return -1;
		}
		//std::cout << "parsed from " << curStartIndex << " to " << (newTokenPosition - 1) << std::endl;
	}
	if (expressionCount > 0) {
		for (unsigned int ci = nextStartIndex; ci < count; ++ci) {
			//std::cout << "finish copy " << ci << std::endl;
			// here std::move() can be used because exprResultValue is replaced at the end
			fullResult.mArray.push_back(std::move(exprResultValue.mArray[ci]));
		}
		// now fullResult is finished --> replace value from parameter
		if (fullResult.mArray.size() == 1) {
			exprResultValue = std::move(fullResult.mArray[0]);
		}
		else {
			exprResultValue = std::move(fullResult);
		}
		return expressionCount;
	}
	return 0;
}

int cfg::interpreter::interpretAndReplace(cfg::Value& cfgValueTree,
		bool allowInterpretationWithQuotes,
		bool allowArrayElementInterpretation,
		bool allowNameInterpretation,
		bool allowValueInterpretation, std::ostream& errMsg)
{
	if (cfgValueTree.isArray()) {
		int rvSum = 0;
		if (allowArrayElementInterpretation) {
			for (Value& element: cfgValueTree.mArray) {
				if (element.isArray() || element.isObject()) {
					int rv = interpretAndReplace(element,
							allowInterpretationWithQuotes,
							allowArrayElementInterpretation,
							allowNameInterpretation,
							allowValueInterpretation, errMsg);
					if (rv == -1) {
						return -1;
					}
					rvSum += rv;
				}
			}
		}
		int rv = interpretAndReplaceExprValue(cfgValueTree,
				allowInterpretationWithQuotes, errMsg);
		if (rv == -1) {
			return -1;
		}
		rvSum += rv;
		return rvSum;
	}
	if (cfgValueTree.isObject()) {
		if (!allowNameInterpretation && !allowValueInterpretation) {
			// --> no interpretation is allowed inside an object --> nothing to do
			return 0;
		}
		int rvSum = 0;
		for (NameValuePair& nvp : cfgValueTree.mObject) {
			if (allowNameInterpretation &&
					(nvp.mName.isArray() || nvp.mName.isObject())) {
				int rv = interpretAndReplace(nvp.mName,
						allowInterpretationWithQuotes,
						allowArrayElementInterpretation,
						allowNameInterpretation,
						allowValueInterpretation, errMsg);
				if (rv == -1) {
					return -1;
				}
				rvSum += rv;
			}
			if (allowValueInterpretation &&
					(nvp.mValue.isArray() || nvp.mValue.isObject())) {
				int rv = interpretAndReplace(nvp.mValue,
						allowInterpretationWithQuotes,
						allowArrayElementInterpretation,
						allowNameInterpretation,
						allowValueInterpretation, errMsg);
				if (rv == -1) {
					return -1;
				}
				rvSum += rv;
			}
		}
		return rvSum;
	}
	return 0;
}

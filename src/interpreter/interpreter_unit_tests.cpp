#include <interpreter/interpreter_unit_tests.h>
#include <interpreter/interpreter.h>
#include <interpreter/cfg_parser.h>
#include <interpreter/lexer.h>
#include <interpreter/cfg_lexer.h>
#include <tml/tml_parser.h>
#include <cfg/cfg_string.h>
#include <iostream>

static int sPassed = 0;
static int sFailed = 0;

/**
 * Parses the given chunk of code and verifies that it matches the expected
 * pretty-printed result.
 * @return 1 for success, 0 for failed
 */
static void test(std::unique_ptr<cfg::TokenIterator> sourceTokens,
		const std::string& expected,
		unsigned int expectedExpressionCount)
{
	std::unique_ptr<cfg::Parser> parser(new cfg::CfgParser(std::move(sourceTokens)));

	std::string actual;
	std::string actualEx;
	unsigned int errorCount = 0;
	unsigned int expressionCount = 0;
	for (;; ++expressionCount) {
		std::unique_ptr<cfg::expressions::Expression> result = parser->parseFullExpressionAllowEmpty(
				errorCount);
		if (result->getExpressionType() == cfg::expressions::ExpressionType::EMPTY) {
			break;
		}

		if (expressionCount > 0) {
			actual += " ";
			actualEx += " ";
		}

		std::stringstream builder;
		result->print(builder);
		actual += builder.str();

		std::stringstream builderEx;
		result->printEx(builderEx);
		actualEx += builderEx.str();
	}

	if (errorCount == 0) {
		if (expected == actual && expressionCount == expectedExpressionCount) {
			std::cout << "[OK]: " << expressionCount << " " << expectedExpressionCount <<
					": '" << expected <<
					"' == '" << actual <<
					"' Ex: '" << actualEx << "'" << std::endl;
			sPassed++;
		} else {
			sFailed++;
			std::cout << "[FAIL] Expected: " << expected << std::endl;
			std::cout << "         Actual: " << actual << std::endl;
			std::cout << "         counts: " << expressionCount << " " << expectedExpressionCount << std::endl;
		}
	} else {
		sFailed++;
		// TODO: print message from ParseErrorExpression
		std::cout << "[FAIL] Expected: " << expected << ", is: " << actual << std::endl;
		std::cout << "    Error count: " << errorCount << std::endl;
		std::cout << "         counts: " << expressionCount << " " << expectedExpressionCount << std::endl;
	}
}

static void test(const std::string& source, const std::string& expected,
		unsigned int expectedExpressionCount = 1)
{
	std::unique_ptr<cfg::Lexer> lexer(new cfg::Lexer(source));
	test(std::move(lexer), expected, expectedExpressionCount);
}

static bool getCfgValueWithArray(const std::string& tmlSource, cfg::Value& exprValue)
{
	cfg::TmlParser parser;
	parser.setStringBuffer("tml-expression", tmlSource);
	cfg::Value value;
	if (!parser.getAsTree(value, true, true)) {
		std::cout << "Parsing tml source failed! source: '" << tmlSource << "'" << std::endl;
		return false;
	}
	//std::cout << "tml input: " << cfg::tmlstring::valueToString(0, value) << std::endl;
	//std::cout << "tml input: " << cfg::cfgstring::valueToString(0, value) << std::endl;
	cfg::Value* val = &value;
	if (val->isObject()) {
		if (val->mObject.empty()) {
			val->setArray();
		}
		else if (val->mObject.size() == 1) {
			val = &val->mObject[0].mName;
		}
	}

	if (val->isEmpty()) {
		val->setArray();
	}
	else if (val->isText()) {
		// convert text to a array with one text entry.
		cfg::Value entry(val->mText);
		val->setArray();
		val->mArray.push_back(entry);
	}

	if (!val->isArray()) {
		std::cout << "No cfg value array." << std::endl;
		return false;
	}
#if 0
	if (val->mArray.empty()) {
		std::cout << "Empty cfg value array is not allowed." << std::endl;
		return nullptr;
	}
#endif
	exprValue = std::move(*val);
	return true;
}

static std::unique_ptr<cfg::CfgLexer> getCfgLexer(const std::string& tmlSource)
{
	cfg::Value value;
	if (!getCfgValueWithArray(tmlSource, value)) {
		return nullptr;
	}
	std::unique_ptr<cfg::CfgLexer> lexer(new cfg::CfgLexer(value, false));
	return lexer;
}

static void testTml(const std::string& tmlSource, const std::string& expected,
		unsigned int expectedExpressionCount = 1)
{
	std::unique_ptr<cfg::CfgLexer> lexer = getCfgLexer(tmlSource);
	if (!lexer) {
		++sFailed;
		return;
	}
	test(std::move(lexer), expected, expectedExpressionCount);
}

static void interpret(std::unique_ptr<cfg::TokenIterator> sourceTokens,
		const std::vector<cfg::Value>& expected,
		bool allowInterpretationWithQuotes)
{
	std::unique_ptr<cfg::Parser> parser(
			new cfg::CfgParser(std::move(sourceTokens)));

	unsigned int expressionCount = 0;
	for (;; ++expressionCount) {
		unsigned int errorCount = 0;
		std::unique_ptr<cfg::expressions::Expression> result = parser->parseFullExpressionAllowEmpty(
				errorCount);
		if (result->getExpressionType() == cfg::expressions::ExpressionType::EMPTY) {
			break;
		}

		if (errorCount > 0) {
			std::cout << "parsing error" << std::endl;
			continue;
		}

		cfg::expressions::Context context(allowInterpretationWithQuotes);
		cfg::Value interpretResult;
		std::stringstream errMsg;
		if (!result->interpret(context, interpretResult, errMsg)) {
			std::cout << "interpret failed! err msg:" << std::endl;
			std::cout << errMsg.str() << std::endl;
			continue;
		}

		result->print(std::cout);
		std::cout << " = " << cfg::tmlstring::valueToString(0, interpretResult);
	}
	if (expressionCount != expected.size()) {
		std::cout << "wrong expression count: " << expressionCount << " != " <<
				expected.size() << std::endl;
	}
}

static void interpretTml(const std::string& tmlSource, const cfg::Value& expected)
{
	std::unique_ptr<cfg::CfgLexer> lexer = getCfgLexer(tmlSource);
	if (!lexer) {
		++sFailed;
		return;
	}
	std::vector<cfg::Value> expectedResults;
	expectedResults.push_back(expected);
	interpret(std::move(lexer), expectedResults, false);
}

static void interpretTml(const std::string& tmlSource, int expected)
{
	cfg::Value expectedValue;
	expectedValue.setInteger(expected);
	interpretTml(tmlSource, expectedValue);
}

static bool testsWithLexer()
{
	sPassed = 0;
	sFailed = 0;

	test("", "", 0);
	test("a b c", "a b c", 3);
	//test("a(+)", "a((+))");
	//test("a)", "a)");
#if 1
	// Function call.
	test("a()", "a()");
	test("a(b)", "a(b)");
	test("a(b, c)", "a(b, c)");
	test("a(b)(c)", "a(b)(c)");
	test("a(b) + c(d)", "(a(b) + c(d))");
	test("a(b ? c : d, e + f)", "a((b ? c : d), (e + f))");

	// Unary precedence.
	test("~!-+a", "(~(!(-(+a))))");
	test("a!!!", "(((a!)!)!)");

	// Unary and binary predecence.
	test("-a * b", "((-a) * b)");
	test("!a + b", "((!a) + b)");
	test("~a ^ b", "((~a) ^ b)");
	test("-a!",    "(-(a!))");
	test("!a!",    "(!(a!))");

	// Binary precedence.
	test("a = b + c * d ^ e - f / g", "(a = ((b + (c * (d ^ e))) - (f / g)))");

	// Binary associativity.
	test("a = b = c", "(a = (b = c))");
	test("a + b - c", "((a + b) - c)");
	test("a * b / c", "((a * b) / c)");
	test("a ^ b ^ c", "(a ^ (b ^ c))");

	// Conditional operator.
	test("a ? b : c ? d : e", "(a ? b : (c ? d : e))");
	test("a ? b ? c : d : e", "(a ? (b ? c : d) : e)");
	test("a + b ? c * d : e / f", "((a + b) ? (c * d) : (e / f))");

	// Grouping.
	test("a + (b + c) + d", "((a + (b + c)) + d)");
	test("a ^ (b + c)", "(a ^ (b + c))");
	test("(!a)!",    "((!a)!)");
#endif

	// Show the results.
	if (sFailed == 0) {
		std::cout << "Passed all " << sPassed << " tests." << std::endl;
	} else {
		std::cout << "----";
		std::cout << "Failed " << sFailed << " out of " +
				(sFailed + sPassed) << " tests." << std::endl;
	}
	return sFailed == 0;
}

static bool testsWithCfgLexer()
{
	sPassed = 0;
	sFailed = 0;

	testTml("", "", 0);
	testTml("a", "a");
	testTml("a b c", "a b c", 3);

#if 0
	// Function call.
	testTml("a( )", "a()");
	testTml("a ( b )", "a(b)");
	testTml("axy ( bxy , cxy )", "axy(bxy, cxy)");
	testTml("a ( b ) ( c )", "a(b)(c)");
	testTml("a ( b ) + c ( d )", "(a(b) + c(d))");
	testTml("a ( b ? c : d , e + f )", "a((b ? c : d), (e + f))");

	// Unary precedence.
	testTml("~ ! - + a", "(~(!(-(+a))))");
	testTml("a ! ! !", "(((a!)!)!)");

	// Unary and binary predecence.
	testTml("- a * b", "((-a) * b)");
	testTml("! a + b", "((!a) + b)");
	testTml("~ a ^ b", "((~a) ^ b)");
	testTml("- a !",    "(-(a!))");
	testTml("! a !",    "(!(a!))");

	// Binary precedence.
	//testTml("a = b + c * d ^ e - f / g", "(a = ((b + (c * (d ^ e))) - (f / g)))");

	// Binary associativity.
	//testTml("a = b = c", "(a = (b = c))");
	testTml("a + b - c", "((a + b) - c)");
	testTml("a * b / c", "((a * b) / c)");
	testTml("a ^ b ^ c", "(a ^ (b ^ c))");

	// Conditional operator.
	testTml("a ? b : c ? d : e", "(a ? b : (c ? d : e))");
	testTml("a ? b ? c : d : e", "(a ? (b ? c : d) : e)");
	testTml("a + b ? c * d : e / f", "((a + b) ? (c * d) : (e / f))");

	// Grouping.
	testTml("a + ( b + c ) + d", "((a + (b + c)) + d)");
	testTml("a ^ ( b + c )", "(a ^ (b + c))");
	testTml("( ! a ) !",    "((!a)!)");
#endif
	// Show the results.
	if (sFailed == 0) {
		std::cout << "Passed all " << sPassed << " tests." << std::endl;
	} else {
		std::cout << "----";
		std::cout << "Failed " << sFailed << " out of " +
				(sFailed + sPassed) << " tests." << std::endl;
	}
	return sFailed == 0;
}

static bool interpretWithCfgLexer()
{
	//interpretTml("1000 -  - 123", 7);
	interpretTml("abs ( - 123 )", 7);
	return true;
}

static bool interpretAndReplace(const std::string& tmlSource)
{
	cfg::Value value;
	if (!getCfgValueWithArray(tmlSource, value)) {
		std::cout << "failed" << std::endl;
		return false;
	}
	int rv = cfg::interpreter::interpretAndReplaceExprValue(value, false);
	std::string str = cfg::tmlstring::valueToString(0, value);
	if (!str.empty() && str[str.size() - 1] == '\n') {
		str.pop_back();
	}
	std::cout << "source: " << tmlSource << std::endl;
	std::cout << "result: " << str << std::endl;
	std::cout << "rv: " << rv << std::endl;
	return true;
}

static bool interpretAndReplaceTests()
{
	interpretAndReplace("123 + 23 + 123");
	interpretAndReplace("a1 _i ( 123 ) aa bb cc dd ee _i ( abc ) zz");
	interpretAndReplace("a1 \"_i\" ( 123 ) aa bb cc dd ee \"_i\" ( abc ) zz");
	interpretAndReplace("a1 b2 c3 d4 _i ( 123 + 23 + 123 ) aa bb _i ( abc + xyz ) zz");
	return true;
}

bool cfg::interpreter::unitTests()
{
	bool rv = true;
	//rv = testsWithLexer() && rv;
	//rv = testsWithCfgLexer() && rv;
	//rv = interpretWithCfgLexer() && rv;
	rv = interpretAndReplaceTests() && rv;
	return rv;
}

#ifndef CFG_CFG_INTERPRETER_H
#define CFG_CFG_INTERPRETER_H

#include <cfg/export.h>
#include <cfg/cfg.h>
#include <interpreter/expressions.h>
#include <memory>

// Parser for interpreter based on a C++ port of Nystrom's bantam demo for pratt parsing.
// Nystrom's bantam: https://github.com/munificent/bantam
// C++ port: https://github.com/jwurzer/bantam-cpp

namespace cfg
{
	namespace interpreter
	{
		/**
		 * Evaluate and interpret the value if one or more expressions are used.
		 * If no expression is used then the value is unchanged and the
		 * function return 0.
		 * @param exprResultValue For input and output.
		 * @param allowInterpretationWithQuotes If source is parsed from a
		 *        JSON file then this should be true.
		 *        If source is parsed from a TML file then this should be false.
		 *        For TML true would also work but then something like this
		 *        is interpreted: "abs" "(" -123 ")"  -->  123
		 *        If false then this would be unchanged.
		 *        If false then the example for interpretation must look like this:
		 *        abs ( -123 )  -->  123
		 * @return -1 for error happened at evaluation and interpretation
		 *         0 for no evaluation and interpretation,
		 *         >0 count for successful evaluation and interpretation
		 */
		CFG_API
		int interpretAndReplaceExprValue(cfg::Value& exprResultValue,
				bool allowInterpretationWithQuotes);

		/**
		 * Interpret and replace the cfgValueTree. If one or more objects
		 * are included then also the name-value-pairs are interpreted
		 * if necessary. If the name and/or the value of a name-value-pair
		 * should be interpreted can be defined with the parameters
		 * allowNameInterpretation and allowValueInterpretation.
		 *
		 * @param cfgValueTree
		 * @param allowInterpretationWithQuotes
		 * @param allowNameInterpretation Defines if a name from
		 *        a name-value-pair of an object should be checked
		 *        for an expression interpretation.
		 * @param allowValueInterpretation Defines if a value from
		 *        a name-value-pair of an object should be checked
		 *        for an expression interpretation.
		 * @return -1 for error happened at evaluation and interpretation
		 *         0 for no evaluation and interpretation,
		 *         >0 count for successful evaluation and interpretation
		 */
		CFG_API
		int interpretAndReplace(cfg::Value& cfgValueTree,
				bool allowInterpretationWithQuotes,
				bool allowNameInterpretation,
				bool allowValueInterpretation);
	}
}

#endif //CFG_CFG_INTERPRETER_H

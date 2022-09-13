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
		 *
		 * @note If one or more expressions are included then the
		 *       exprResultValue can only be an array. If exact one expression
		 *       is inside then the result would be an array with on element
		 *       (the result of the expression). In this case the array is
		 *       removed and the result is directly stored.
		 *
		 * @note This function doesn't support expressions inside an array as
		 *       element-array (array inside an array). Also an object with
		 *       expressions in there name-value-pairs are ignored. For
		 *       this case the function interpretAndReplace() exist.
		 *
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
				bool allowInterpretationWithQuotes, std::ostream& errMsg);

		/**
		 * Interpret and replace the cfgValueTree. If one or more objects
		 * are included then also the name-value-pairs are interpreted
		 * if necessary. If the name and/or the value of a name-value-pair
		 * should be interpreted can be defined with the parameters
		 * allowNameInterpretation and allowValueInterpretation.
		 * Also an array of an expression which is inside another array as
		 * element would be interpreted if allowArrayElementInterpretation
		 * is true.
		 *
		 * @param cfgValueTree The value tree with its expressions.
		 * @param allowInterpretationWithQuotes If source is parsed from a
		 *        JSON file then this should be true.
		 *        If source is parsed from a TML file then this should be false.
		 *        For TML true would also work but then something like this
		 *        is interpreted: "abs" "(" -123 ")"  -->  123
		 *        If false then this would be unchanged.
		 *        If false then the example for interpretation must look like this:
		 *        abs ( -123 )  -->  123
		 * @param allowArrayElementInterpretation If true then an expression
		 *        as an array inside another array is allowed and would be
		 *        interpreted. The array of the expression is stored as
		 *        element inside another expression.
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
				bool allowArrayElementInterpretation,
				bool allowNameInterpretation,
				bool allowValueInterpretation,
				std::ostream& errMsg);
	}
}

#endif //CFG_CFG_INTERPRETER_H

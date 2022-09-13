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
		 * @return -1 for error happened at evaluation and interpretation
		 *         0 for no evaluation and interpretation,
		 *         1 for successful evaluation and interpretation
		 */
		CFG_API
		int interpretAndStore(cfg::Value& exprResultValue,
				bool allowInterpretationWithQuotes);

		std::unique_ptr<expressions::Expression> getAbstractSyntaxTree(const cfg::Value& expressionValue);
	}
}

#endif //CFG_CFG_INTERPRETER_H

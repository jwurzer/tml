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
		//int evaluateAndStore(cfg::Value& exprResultValue);

		std::unique_ptr<expressions::Expression> getAbstractSyntaxTree(const cfg::Value& expressionValue);

		bool unitTests();
	}
}

#endif //CFG_CFG_INTERPRETER_H

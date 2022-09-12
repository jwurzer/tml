#ifndef CFG_PARSELETS_H
#define CFG_PARSELETS_H

#include <interpreter/token.h>
#include <memory>

namespace cfg
{
	class Parser;

	namespace expressions
	{
		class Expression;
	}

	namespace parselets
	{
		/**
		 * One of the two parselet interfaces used by the Pratt parser. An
		 * InfixParselet is associated with a token that appears in the middle of the
		 * expression it parses. Its parse() method will be called after the left-hand
		 * side has been parsed, and it in turn is responsible for parsing everything
		 * that comes after the token. This is also used for postfix expressions, in
		 * which case it simply doesn't consume any more tokens in its parse() call.
		 */
		class InfixParselet {
		public:
			virtual ~InfixParselet() = default;
			virtual std::unique_ptr<expressions::Expression> parse(Parser& parser,
					std::unique_ptr<expressions::Expression> left, Token token,
					unsigned int& errorCount) const = 0;
			virtual int getPrecedence() const = 0;
		};

		/**
		 * One of the two interfaces used by the Pratt parser. A PrefixParselet is
		 * associated with a token that appears at the beginning of an expression. Its
		 * parse() method will be called with the consumed leading token, and the
		 * parselet is responsible for parsing anything that comes after that token.
		 * This interface is also used for single-token expressions like variables, in
		 * which case parse() simply doesn't consume any more tokens.
		 */
		class PrefixParselet {
		public:
			virtual ~PrefixParselet() = default;
			virtual std::unique_ptr<expressions::Expression> parse(
					Parser& parser, Token token, unsigned int& errorCount) const = 0;
		};
	}
}

#endif

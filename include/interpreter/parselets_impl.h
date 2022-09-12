#ifndef CFG_PARSELETS_IMPL_H
#define CFG_PARSELETS_IMPL_H

#include <interpreter/parselets.h>
#include <interpreter/precedence.h>
#include <interpreter/expressions.h>
#include <interpreter/parser.h>

namespace cfg
{
	namespace parselets
	{
		////////////////// InfixParselet implementations //////////////////

		/**
		 * Generic infix parselet for a binary arithmetic operator. The only
		 * difference when parsing, "+", "-", "*", "/", and "^" is precedence and
		 * associativity, so we can use a single parselet class for all of those.
		 */
		class BinaryOperatorParselet: public InfixParselet {
		public:
			BinaryOperatorParselet(int precedence, bool isRight)
					:mPrecedence(precedence),
					mIsRight(isRight)
			{
			}

			virtual std::unique_ptr<expressions::Expression> parse(Parser& parser,
					std::unique_ptr<expressions::Expression> left, Token token,
					unsigned int& errorCount) const override {
				// To handle right-associative operators like "^", we allow a slightly
				// lower precedence when parsing the right-hand side. This will let a
				// parselet with the same precedence appear on the right, which will then
				// take *this* parselet's result as its left-hand argument.
				std::unique_ptr<expressions::Expression> right = parser.parseExpression(
						mPrecedence - (mIsRight ? 1 : 0), errorCount);

				return std::unique_ptr<expressions::OperatorExpression>(
						new expressions::OperatorExpression(std::move(left),
								token.getType(), std::move(right)));
			}

			virtual int getPrecedence() const override {
				return mPrecedence;
			}

		private:
			const int mPrecedence;
			const bool mIsRight;
		};

		/**
		 * Parselet to parse a function call like "a(b, c, d)".
		 */
		class CallParselet: public InfixParselet {
			virtual std::unique_ptr<expressions::Expression> parse(Parser& parser,
					std::unique_ptr<expressions::Expression> left,
					Token /*token*/, unsigned int& errorCount) const override {
				// Parse the comma-separated arguments until we hit, ")".
				std::list<std::unique_ptr<expressions::Expression>> args;

				// There may be no arguments at all.
				if (!parser.match(TokenType::RIGHT_PAREN)) {
					do {
						args.push_back(parser.parseFullExpression(errorCount));
					} while (parser.match(TokenType::COMMA));
					bool err = false;
					std::string errMsg;
					parser.consume(TokenType::RIGHT_PAREN, err, errMsg);
					if (err) {
						++errorCount;
						return std::unique_ptr<expressions::ParseErrorExpression>(
								new expressions::ParseErrorExpression(errMsg));
					}
				}

				return std::unique_ptr<expressions::CallExpression>(
						new expressions::CallExpression(std::move(left), args));
			}

			virtual int getPrecedence() const override {
				return precedence::CALL;
			}
		};

		/**
		 * Parselet for the condition or "ternary" operator, like "a ? b : c".
		 */
		class ConditionalParselet: public InfixParselet {
			virtual std::unique_ptr<expressions::Expression> parse(Parser& parser,
					std::unique_ptr<expressions::Expression> left,
					Token /*token*/, unsigned int& errorCount) const override {
				std::unique_ptr<expressions::Expression> thenArm = parser.parseFullExpression(errorCount);
				bool err = false;
				std::string errMsg;
				parser.consume(TokenType::COLON, err, errMsg);
				if (err) {
					++errorCount;
					return std::unique_ptr<expressions::ParseErrorExpression>(
							new expressions::ParseErrorExpression(errMsg));
				}
				std::unique_ptr<expressions::Expression> elseArm =
						parser.parseExpression(precedence::CONDITIONAL - 1, errorCount);

				return std::unique_ptr<expressions::ConditionalExpression>(
						new expressions::ConditionalExpression(std::move(left), std::move(thenArm), std::move(elseArm)));
			}

			virtual int getPrecedence() const override {
				return precedence::CONDITIONAL;
			}
		};

		/**
		 * Generic infix parselet for an unary arithmetic operator. Parses postfix
		 * unary "?" expressions.
		 */
		class PostfixOperatorParselet: public InfixParselet {
		public:
			PostfixOperatorParselet(int precedence)
					:mPrecedence(precedence)
			{
			}

			virtual std::unique_ptr<expressions::Expression> parse(Parser& /*parser*/,
					std::unique_ptr<expressions::Expression> left, Token token,
					unsigned int& /*errorCount*/) const override {
				return std::unique_ptr<expressions::PostfixExpression>(
						new expressions::PostfixExpression(std::move(left), token.getType()));
			}

			virtual int getPrecedence() const override {
				return mPrecedence;
			}

		private:
			const int mPrecedence;
		};

		////////////////// PrefixParselet implementations //////////////////

		/**
		 * Parses parentheses used to group an expression, like "a * (b + c)".
		 */
		class GroupParselet: public PrefixParselet {
			virtual std::unique_ptr<expressions::Expression> parse(
					Parser& parser, Token /*token*/, unsigned int& errorCount) const override {
				std::unique_ptr<expressions::Expression> expression = parser.parseFullExpression(errorCount);
				bool err = false;
				std::string errMsg;
				parser.consume(TokenType::RIGHT_PAREN, err, errMsg);
				if (err) {
					++errorCount;
					return std::unique_ptr<expressions::ParseErrorExpression>(
							new expressions::ParseErrorExpression(errMsg));
				}
				return expression;
			}
		};

		/**
		 * Simple parselet for a named variable like "abc".
		 */
		class ValueParselet: public PrefixParselet {
			virtual std::unique_ptr<expressions::Expression> parse(
					Parser& /*parser*/, Token token,
					unsigned int& /*errorCount*/) const override {
				return std::unique_ptr<expressions::ValueExpression>(
						new expressions::ValueExpression(token.getValue()));
			}
		};

		/**
		 * Generic prefix parselet for an unary arithmetic operator. Parses prefix
		 * unary "-", "+", "~", and "!" expressions.
		 */
		class PrefixOperatorParselet: public PrefixParselet {
		public:
			PrefixOperatorParselet(int precedence)
					:mPrecedence(precedence)
			{
			}

			virtual std::unique_ptr<expressions::Expression> parse(
					Parser& parser, Token token, unsigned int& errorCount) const override {
				// To handle right-associative operators like "^", we allow a slightly
				// lower precedence when parsing the right-hand side. This will let a
				// parselet with the same precedence appear on the right, which will then
				// take *this* parselet's result as its left-hand argument.
				std::unique_ptr<expressions::Expression> right = parser.parseExpression(mPrecedence, errorCount);

				return std::unique_ptr<expressions::PrefixExpression>(
						new expressions::PrefixExpression(token.getType(), std::move(right)));
			}

			// not necessary. PrefixParselet doesn't have abstract function getPrecedence().
			//int getPrecedence() const {
			//	return mPrecedence;
			//}

		private:
			const int mPrecedence;
		};
	}
}
#endif

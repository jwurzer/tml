#ifndef CFG_CFG_PARSER_H
#define CFG_CFG_PARSER_H

#include <interpreter/parser.h>
#include <interpreter/parselets.h>

namespace cfg
{
	/**
	 * Extends the generic Parser class with support for parsing the actual Bantam
	 * grammar.
	 */
	class CfgParser: public Parser {
	public:
		CfgParser(std::unique_ptr<TokenIterator> tokens)
			:Parser(std::move(tokens))
		{
			// Register all of the parselets for the grammar.

			// Register the ones that need special parselets.
			registerParselet(TokenType::VALUE,       std::unique_ptr<parselets::ValueParselet>(new parselets::ValueParselet()));
			registerParselet(TokenType::QUESTION,   std::unique_ptr<parselets::ConditionalParselet>(new parselets::ConditionalParselet()));
			registerParselet(TokenType::LEFT_PAREN, std::unique_ptr<parselets::GroupParselet>(new parselets::GroupParselet()));
			registerParselet(TokenType::LEFT_PAREN, std::unique_ptr<parselets::CallParselet>(new parselets::CallParselet()));

			// Register the simple operator parselets.
			prefix(TokenType::PLUS,      precedence::PREFIX);
			prefix(TokenType::MINUS,     precedence::PREFIX);

			infixLeft(TokenType::PLUS,     precedence::SUM);
			infixLeft(TokenType::MINUS,    precedence::SUM);
			infixLeft(TokenType::ASTERISK, precedence::PRODUCT);
			infixLeft(TokenType::SLASH,    precedence::PRODUCT);
		}

		/**
		 * Registers a postfix unary operator parselet for the given token and
		 * precedence.
		 */
		void postfix(TokenType token, int precedence) {
			registerParselet(token, std::unique_ptr<parselets::PostfixOperatorParselet>(
					new parselets::PostfixOperatorParselet(precedence)));
		}

		/**
		 * Registers a prefix unary operator parselet for the given token and
		 * precedence.
		 */
		void prefix(TokenType token, int precedence) {
			registerParselet(token, std::unique_ptr<parselets::PrefixOperatorParselet>(
					new parselets::PrefixOperatorParselet(precedence)));
		}

		/**
		 * Registers a left-associative binary operator parselet for the given token
		 * and precedence.
		 */
		void infixLeft(TokenType token, int precedence) {
			registerParselet(token, std::unique_ptr<parselets::BinaryOperatorParselet>(
					new parselets::BinaryOperatorParselet(precedence, false)));
		}

		/**
		 * Registers a right-associative binary operator parselet for the given token
		 * and precedence.
		 */
		void infixRight(TokenType token, int precedence) {
			registerParselet(token, std::unique_ptr<parselets::BinaryOperatorParselet>(
					new parselets::BinaryOperatorParselet(precedence, true)));
		}
	};
}

#endif

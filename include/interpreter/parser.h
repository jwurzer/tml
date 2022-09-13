#ifndef CFG_PARSER_H
#define CFG_PARSER_H

#include <interpreter/token_iterator.h>
#include <interpreter/expressions.h>
#include <interpreter/parselets.h>

#include <vector>
#include <map>

namespace cfg
{
	class Parser {
	public:
		Parser(std::unique_ptr<TokenIterator> tokens)
			:mTokens(std::move(tokens))
		{
		}

		virtual ~Parser() = default;

		void registerParselet(TokenType token, std::unique_ptr<parselets::PrefixParselet> parselet) {
			mPrefixParselets[token] = std::move(parselet);
		}

		void registerParselet(TokenType token, std::unique_ptr<parselets::InfixParselet> parselet) {
			mInfixParselets[token] = std::move(parselet);
		}

		/**
		 * @param precedence Precedence which is NOT allowed. Encountered
		 * expression must have a precedence which is higher than this parameter.
		 * This parameter is a number that tells which expressions can be
		 * parsed by that call. Every encountered expression which has a
		 * higher precedence than this parameter is added.
		 * If parseExpression() encounters an expression
		 * whose precedence is lower OR EQUAL than this parameter, it stops parsing and
		 * returns what it has so far.
		 */
		std::unique_ptr<expressions::Expression> parseExpressionEx(int precedence,
				bool allowEOF, unsigned int& errorCount) {
			Token token = consume();
			if (token.getType() == TokenType::END_OF_FILE) {
				if (allowEOF) {
					// Don't increase errorCount because its not error. its allowed.
					return std::unique_ptr<expressions::EmptyExpression>(
							new expressions::EmptyExpression());
				}
				++errorCount;
				return std::unique_ptr<expressions::ParseErrorExpression>(
						new expressions::ParseErrorExpression(
								"Could not parse \"" + token.getText() + "\". EOF!"));
			}
			auto it = mPrefixParselets.find(token.getType());

			if (it == mPrefixParselets.end()) {
				++errorCount;
				return std::unique_ptr<expressions::ParseErrorExpression>(
						new expressions::ParseErrorExpression(
								"Could not parse \"" + token.getText() + "\"."));
			}

			parselets::PrefixParselet* prefix = it->second.get();

			std::unique_ptr<expressions::Expression> left = prefix->parse(*this, token, errorCount);

			while (precedence < getPrecedence()) { // same as while (getPrecedence() > precedence) {
				token = consume();

				parselets::InfixParselet* infix = mInfixParselets[token.getType()].get();
				left = infix->parse(*this, std::move(left), token, errorCount);
			}

			return left;
		}

		std::unique_ptr<expressions::Expression> parseExpression(int precedence, unsigned int& errorCount) {
			return parseExpressionEx(precedence, false, errorCount);
		}

		std::unique_ptr<expressions::Expression> parseFullExpression(unsigned int& errorCount) {
			return parseExpressionEx(0, false, errorCount);
		}

		std::unique_ptr<expressions::Expression> parseFullExpressionAllowEmpty(unsigned int& errorCount) {
			return parseExpressionEx(0, true, errorCount);
		}

		bool match(TokenType expected) {
			Token token = lookAhead(0);
			if (token.getType() != expected) {
				return false;
			}

			consume();
			return true;
		}

		Token consume(TokenType expected, bool& error, std::string& errorMessage) {
			Token token = lookAhead(0);
			if (token.getType() != expected) {
				error = true;
				errorMessage = "Expected token " + tokentype::toString(expected) +
						" and found " + tokentype::toString(token.getType());
				return token;
			}

			return consume();
		}

		Token consume() {
			// Make sure we've read the token.
			lookAhead(0);

			Token front = mRead.front();
			mRead.erase(mRead.begin());
			return front;
		}

		TokenIterator& getTokenIterator() { return *mTokens; }

		void reset() {
			mRead.clear();
		}
	private:
		Token lookAhead(unsigned int distance) {
			// Read in as many as needed.
			while (distance >= mRead.size()) {
				mRead.push_back(mTokens->next());
			}

			// Get the queued token.
			return mRead[distance];
		}

		int getPrecedence() {
			auto itParser = mInfixParselets.find(lookAhead(0).getType());
			if (itParser != mInfixParselets.end()) return itParser->second->getPrecedence();

			return 0;
		}

		const std::unique_ptr<TokenIterator> mTokens;
		std::vector<Token> mRead;
		std::map<TokenType, std::unique_ptr<parselets::PrefixParselet>> mPrefixParselets;
		std::map<TokenType, std::unique_ptr<parselets::InfixParselet>> mInfixParselets;
	};

}

#endif

#ifndef CFG_TOKEN_TYPE_H
#define CFG_TOKEN_TYPE_H

#include <string>

namespace cfg
{

	enum class TokenType {
		LEFT_PAREN = 0,
		RIGHT_PAREN,
		COMMA,
		PLUS,
		MINUS,
		ASTERISK,
		SLASH,
		QUESTION,
		COLON,
		VALUE,
		END_OF_FILE, // should be the last one
	};

	namespace tokentype
	{
		/**
		 * If the TokenType represents a punctuator (i.e. a token that can split an
		 * identifier like '+', this will get its text.
		 */
		static inline char punctuator(TokenType tokenType) {
			switch (tokenType) {
			case TokenType::LEFT_PAREN:  return '(';
			case TokenType::RIGHT_PAREN: return ')';
			case TokenType::COMMA:       return ',';
			case TokenType::PLUS:        return '+';
			case TokenType::MINUS:       return '-';
			case TokenType::ASTERISK:    return '*';
			case TokenType::SLASH:       return '/';
			case TokenType::QUESTION:    return '?';
			case TokenType::COLON:       return ':';
			default:                          return '\0';
			}
		}

		static inline std::string toString(TokenType tokenType) {
			switch (tokenType) {
			case TokenType::LEFT_PAREN: return "LEFT_PAREN";
			case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
			case TokenType::COMMA: return "COMMA";
			case TokenType::PLUS: return "PLUS";
			case TokenType::MINUS: return "MINUS";
			case TokenType::ASTERISK: return "ASTERISK";
			case TokenType::SLASH: return "SLASH";
			case TokenType::QUESTION: return "QUESTION";
			case TokenType::COLON: return "COLON";
			case TokenType::VALUE: return "VALUE";
			case TokenType::END_OF_FILE: return "END_OF_FILE";
			}
			return "unknown";
		}
	}
}

#endif
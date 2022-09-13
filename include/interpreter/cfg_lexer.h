#ifndef CFG_CFG_LEXER_H
#define CFG_CFG_LEXER_H

#include <interpreter/token_iterator.h>
#include <cfg/cfg.h>
#include <map>
#include <string>

namespace cfg
{
	/**
	 * A very primitive lexer. Takes a string and splits it into a series of
	 * Tokens. Operators and punctuation are mapped to unique keywords. Names,
	 * which can be any series of letters, are turned into NAME tokens. All other
	 * characters are ignored (except to separate names). Numbers and strings are
	 * not supported. This is really just the bare minimum to give the parser
	 * something to work with.
	 */
	class CfgLexer: public TokenIterator {
	public:
		/**
		 * Creates a new Lexer to tokenize the given string.
		 * @param text String to tokenize.
		 */
		CfgLexer(const cfg::Value& expressionValue, bool allowInterpretationWithQuotes)
			:mTokenValues(expressionValue.mArray),
			mAllowInterpretationWithQuotes(allowInterpretationWithQuotes),
			mIndex(0), mOutOfRangeIndex(mTokenValues.size())
		{
			// Register all of the TokenTypes that are explicit punctuators.
			for (int tt = 0; tt <= static_cast<int>(TokenType::END_OF_FILE); ++tt) {
				TokenType type = static_cast<TokenType>(tt);
				char punctuator = tokentype::punctuator(type);
				if (punctuator != '\0') {
					mPunctuators[punctuator] = type;
				}
			}
		}

		virtual bool hasNext() const override {
			return true;
		}

		virtual Token next() override {
			while (mIndex < mOutOfRangeIndex) {
				const Value& val = mTokenValues[mIndex++];
				char c = (val.isText() &&
						(mAllowInterpretationWithQuotes || !val.mParseTextWithQuotes) &&
						val.mText.size() == 1) ? val.mText[0] : '\0';
				if (mPunctuators.count(c) > 0) {
					// Handle punctuation.
					return Token(mPunctuators.at(c), c);
				} else if (!val.isEmpty() && !val.isComment()) {
					// Handle names.
					return Token(TokenType::VALUE, val);
				} else {
					// Ignore all other characters (whitespace, etc.)
				}
			}

			// Once we've reached the end of the string, just return EOF tokens. We'll
			// just keeping returning them as many times as we're asked so that the
			// parser's lookahead doesn't have to worry about running out of tokens.
			return Token(TokenType::END_OF_FILE, Value{});
		}

		virtual bool movePosition(unsigned int absolutePositionIndex) override {
			mIndex = absolutePositionIndex;
			return true;
		}
		virtual bool setRangePosition(unsigned int beginIndex, unsigned int outOfRangeIndex) override {
			if (beginIndex > outOfRangeIndex || outOfRangeIndex > mTokenValues.size()) {
				return false;
			}
			mIndex = beginIndex;
			mOutOfRangeIndex = outOfRangeIndex;
			return true;
		}

		virtual unsigned int getPosition() const override { return mIndex; }

	private:
		std::map<char, TokenType> mPunctuators;
		std::vector<Value> mTokenValues;
		bool mAllowInterpretationWithQuotes;
		unsigned int mIndex;
		unsigned int mOutOfRangeIndex;
	};
}

#endif
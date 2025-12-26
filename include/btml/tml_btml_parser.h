#ifndef CFG_TML_BTML_PARSER_H
#define CFG_TML_BTML_PARSER_H

#include <tml/tml_parser.h>
#include <btml/btml_parser.h>

namespace cfg
{
	/**
	 * Supports tml and btml parsing.
	 * Internal it uses a tml parser and a btml parser.
	 */
	class CFG_API TmlBtmlParser: public ValueParser
	{
	public:
		TmlBtmlParser();
		TmlBtmlParser(const std::string& filename);
		virtual ~TmlBtmlParser();
		virtual void reset() override;
		virtual bool setFilename(const std::string& filename) override;
		virtual bool getAsTree(Value& root,
				bool inclEmptyLines, bool inclComments) override;
		// return filename with linenumber and error message
		virtual std::string getExtendedErrorMsg() const override;
	protected:
		// 0 for none, 1 for tml parser, 2 for btml parser
		static int getParserSelectionForFilename(const std::string& filename,
				std::string& outErrorMsg);
	private:
		std::string mFilename;
		// current parser selection
		// 0 for none, 1 for tml parser, 2 for btml parser
		int mCurrentParserSelection = 0;
		TmlParser mTmlParser;
		BtmlParser mBtmlParser;
		std::string mErrorMsg;
	};
}

#endif

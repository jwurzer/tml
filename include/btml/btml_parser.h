#ifndef CFG_BTML_PARSER_H
#define CFG_BTML_PARSER_H

#include <cfg/export.h>
#include <cfg/value_parser.h>
#include <vector>
#include <stdint.h>

namespace cfg
{
	class NameValuePair;
	class Value;

	class CFG_API BtmlParser: public ValueParser
	{
	public:
		BtmlParser();
		BtmlParser(const std::string& filename);
		virtual ~BtmlParser();
		virtual void reset() override;
		virtual bool setFilename(const std::string& filename) override;
		// inclEmptyLines and inclComments parameter are ignored!
		virtual bool getAsTree(Value& root,
				bool inclEmptyLines, bool inclComments) override;
		// return filename with linenumber and error message
		virtual std::string getExtendedErrorMsg() const override;
	private:
		std::string mFilename;
		std::vector<uint8_t> mData;
		bool mDataIsValid = false;
		std::string mErrorMsg;

		bool loadDataFromFile();
	};
}

#endif

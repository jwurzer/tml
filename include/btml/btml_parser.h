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
		enum class Source
		{
			NONE = 0,
			FILE,
			STRING_STREAM,
			CUSTOM_BUFFER,
		};
		BtmlParser();
		BtmlParser(const std::string& filename);
		virtual ~BtmlParser();
		virtual void reset() override;
		virtual bool setFilename(const std::string& filename) override;
		// strBuffer must contain the btml binary format!!!
		bool setStringBuffer(const std::string& pseudoFilename,
				const std::string& strBuffer);
		// If this is used then only the pointer is copied!
		// --> buf must live as long as BtmlParser object!
		bool setCustomBuffer(const std::string& pseudoFilename,
				const uint8_t* buf, unsigned int bufSize);
		// inclEmptyLines and inclComments parameter are ignored!
		virtual bool getAsTree(Value& root,
				bool inclEmptyLines, bool inclComments) override;
		// return filename with linenumber and error message
		virtual std::string getExtendedErrorMsg() const override;
	private:
		Source mSource = Source::NONE;
		std::string mFilename;
		const uint8_t* mBuf = nullptr;
		unsigned int mBufSize = 0;
		std::vector<uint8_t> mData;
		bool mDataIsValid = false; // if true then mData or mBuf/mBufSize (CUSTOM_BUFFER) is valid.
		std::string mErrorMsg;

		bool loadDataFromFile();
	};
}

#endif

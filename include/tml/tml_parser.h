#ifndef CFG_TML_PARSER_H
#define CFG_TML_PARSER_H

#include <cfg/export.h>
#include <cfg/value_parser.h>

#include <fstream>
#include <sstream>
#include <string>

namespace cfg
{
	class NameValuePair;
	class Value;

	/**
	 * TML - Tiny Markup Language
	 * See more infos at the end of this file.
	 */
	class CFG_API TmlParser: public ValueParser
	{
	public:
		enum class Source
		{
			NONE = 0,
			FILE,
			STRING_STREAM,
			CUSTOM_STREAM,
		};
		TmlParser();
		TmlParser(const std::string& filename);
		virtual ~TmlParser();
		virtual void reset() override;
		virtual bool setFilename(const std::string& filename) override;
		bool setStringBuffer(const std::string& pseudoFilename,
				const std::string& strBuffer);
		// If this is used then only the pointer is copied!
		// --> inStream must live as long as TmlParser object!
		bool setCustomStream(const std::string& pseudoFilename,
				std::istream* inStream);
		bool begin();
		// return -1 for error, -2 for end of file or deep count for success
		int getNextTmlEntry(NameValuePair& entry);
		int getNextTmlEntry(NameValuePair& entry, std::string* outLine, int* outLineNumber);
		// return -1 for error or deep count for success
		int getNextTmlEntry(std::string& utf8Line, NameValuePair& entry,
				int lineNumber);
		bool getAsTree(NameValuePair &root,
				bool inclEmptyLines = false, bool inclComments = false);
		virtual bool getAsTree(Value &root,
				bool inclEmptyLines = false, bool inclComments = false) override;
		const std::string& getErrorMsg() const { return mErrorMsg; }
		unsigned int getLineNumber() const { return mLineNumber; }
		// return filename with linenumber and error message
		virtual std::string getExtendedErrorMsg() const override;
		int getErrorCode() const { return mErrorCode; }
	private:
		Source mSource;
		std::string mFilename;
		std::string mStrBuffer;
		std::ifstream mIfs;
		std::istringstream mIss;
		std::istream* mInStream;
		std::string mLine;
		int mErrorCode;
		std::string mErrorMsg;
		unsigned int mLineNumber;

		char mIndentChar;
		unsigned int mIndentCharCount;
	};

	namespace tmlparser
	{
		// helper functions to convert a tml string to a cfg::Value.
		CFG_API
		Value getValueFromString(const std::string& tml,
				bool inclEmptyLines = false, bool inclComments = false,
				std::string* errMsg = nullptr);

		CFG_API
		bool getValueFromString(cfg::Value& outValue, const std::string& tml,
				bool inclEmptyLines = false, bool inclComments = false,
				std::string* errMsg = nullptr);
	}
}

#endif

/*
 * TML - Tiny Markup Language
 *
 * The first indent defines the indent character and the count of this character.
 * The indent can be one or more tabs or one or more spaces.
 * Mixing is not allowed!
 *
 * Other (similar) markup language file formats
 * https://en.wikipedia.org/wiki/JSON
 * https://en.wikipedia.org/wiki/YAML
 * https://en.wikipedia.org/wiki/INI_file
 * https://en.wikipedia.org/wiki/TOML
 * https://de.wikipedia.org/wiki/Extensible_Markup_Language
 */
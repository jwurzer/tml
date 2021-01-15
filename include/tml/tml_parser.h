#ifndef CFG_TML_PARSER_H
#define CFG_TML_PARSER_H

#include <cfg/export.h>

#include <fstream>
#include <string>

namespace cfg
{
	class NameValuePair;
	class Value;

	/**
	 * TML - Tiny Markup Language
	 * See more infos at the end of this file.
	 */
	class CFG_API TmlParser
	{
	public:
		TmlParser();
		TmlParser(const std::string& filename);
		~TmlParser();
		void reset();
		bool setFilename(const std::string& filename);
		bool begin();
		// return -1 for error, -2 for end of file or deep count for success
		int getNextTmlEntry(NameValuePair& entry);
		// return -1 for error or deep count for success
		int getNextTmlEntry(std::string& utf8Line, NameValuePair& entry,
				int lineNumber);
		bool getAsTree(NameValuePair &root,
				bool inclEmptyLines = false, bool inclComments = false);
		bool getAsTree(Value &root,
				bool inclEmptyLines = false, bool inclComments = false);
		const std::string& getErrorMsg() const { return mErrorMsg; }
		unsigned int getLineNumber() const { return mLineNumber; }
		// return filename with linenumber and error message
		std::string getExtendedErrorMsg() const;
	private:
		std::string mFilename;
		std::ifstream mIfs;
		std::string mLine;
		int mErrorCode;
		std::string mErrorMsg;
		unsigned int mLineNumber;

		char mIndentChar;
		unsigned int mIndentCharCount;
	};
}

#endif /* GLSLSCENE_SML_PARSER_H */

/*
 * SML - Smart Markup Language
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
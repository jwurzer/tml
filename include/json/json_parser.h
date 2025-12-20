#ifndef CFG_JSON_PARSER_H
#define CFG_JSON_PARSER_H

#include <cfg/export.h>
#include <cfg/value_parser.h>

#include <string>

namespace cfg
{
	class NameValuePair;
	class Value;

	/**
	 * JSON - JavaScript Object Notation
	 *
	 * A JSON parser which internal uses JSMN
	 * https://zserge.com/jsmn/
	 * https://github.com/zserge/jsmn
	 *
	 * The file jsmn.h is MIT license and from the git repository
	 * https://github.com/zserge/jsmn
	 * commit 053d3cd29200edb1bfd181d917d140c16c1f8834
	 * Date: Thu Apr 2 2020
	 * If no API changes happened then the file jsmn.h could be replaced
	 * with the current available version.
	 *
	 * For JSON standard see:
	 * https://www.ecma-international.org/publications-and-standards/standards/ecma-404/
	 * https://www.ecma-international.org/wp-content/uploads/ECMA-404_2nd_edition_december_2017.pdf
	 */
	class CFG_API JsonParser: public ValueParser
	{
	public:
		JsonParser();
		JsonParser(const std::string& filename);
		virtual ~JsonParser();
		virtual void reset() override;
		virtual bool setFilename(const std::string& filename) override;
		bool getAsTree(NameValuePair &root);
		bool getAsTree(Value &root);
		// inclEmptyLines and inclComments parameter are ignored!
		virtual bool getAsTree(Value& root,
				bool inclEmptyLines, bool inclComments) override;
		static bool getAsTree(Value &root, const std::string& filename,
				unsigned int& outLineNumber, std::string& outErrorMsg);
		static bool getAsTree(Value &root, const std::string& filenameInfo,
				std::istream& stream, unsigned int& outLineNumber,
				std::string& outErrorMsg);
		const std::string& getErrorMsg() const { return mErrorMsg; }
		unsigned int getLineNumber() const { return mLineNumber; }
		// return filename with linenumber and error message
		virtual std::string getExtendedErrorMsg() const override;
	private:
		std::string mFilename;
		std::string mErrorMsg;
		unsigned int mLineNumber;
	};
}

#endif

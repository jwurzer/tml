#ifndef CFG_JSON_PARSER_H
#define CFG_JSON_PARSER_H

#include <cfg/export.h>

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
	 */
	class CFG_API JsonParser
	{
	public:
		JsonParser();
		JsonParser(const std::string& filename);
		~JsonParser();
		void reset();
		bool setFilename(const std::string& filename);
		bool getAsTree(NameValuePair &root);
		bool getAsTree(Value &root);
		static bool getAsTree(Value &root, const std::string& filename,
				unsigned int& outLineNumber, std::string& outErrorMsg);
		const std::string& getErrorMsg() const { return mErrorMsg; }
		unsigned int getLineNumber() const { return mLineNumber; }
		// return filename with linenumber and error message
		std::string getExtendedErrorMsg() const;
	private:
		std::string mFilename;
		std::string mErrorMsg;
		unsigned int mLineNumber;
	};
}

#endif

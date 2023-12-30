#ifndef CFG_VALUE_PARSER_H
#define CFG_VALUE_PARSER_H

#include <cfg/export.h>
#include <string>

namespace cfg
{
	class Value;

	/**
	 * Parse from data (file) a cfg::Value (and its children).
	 */
	class CFG_API ValueParser
	{
	public:
		virtual ~ValueParser() = default;
		virtual void reset() = 0;
		virtual bool setFilename(const std::string& filename) = 0;
		virtual bool getAsTree(Value& root,
				bool inclEmptyLines = false, bool inclComments = false) = 0;
		// return filename with linenumber and error message
		virtual std::string getExtendedErrorMsg() const = 0;
	};
}

#endif

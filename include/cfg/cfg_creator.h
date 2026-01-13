#ifndef CFG_CFG_CREATOR_H
#define CFG_CFG_CREATOR_H

#include <cfg/export.h>
#include <cfg/cfg.h>
#include <string>

namespace cfg
{
	/**
	 * someText = hello
	 * a-number = 13
	 * perentage = 0.34
	 * array-with-names = Alice Bob Carol Eve Mallory
	 * aObjectWithTml
	 *    name = Max
	 *    weight = 71.7
	 *    body-height = 123
	 * use-mipmap = false
	 * active = true
	 * asdf = null
	 * # this is a comment
	 *
	 * CfgCreator cc;
	 * cc
	 *    .nvpText("someText", "hello")
	 *    .nvpInt("a-number", 13)
	 *    .nvpFloat("percentage", 0.34f)
	 *    .pushArray("array-with-names")
	 *       .valText("Alice")
	 *       .valText("Bob")
	 *       .valText("Carol")
	 *       .valText("Eve")
	 *       .valText("Mallory")
	 *    .popArray()
	 *    .pushObject("aObjectWithTml")
	 *       .nvpText("name", "Max")
	 *       .nvpFloat("weight", 71.7f)
	 *       .nvpInt("body-height", 123)
	 *    .popObject()
	 *    .nvpBool("use-mipmap", false)
	 *    .nvpBool("active", true)
	 *    .nvpNull("asdf")
	 *    .comment("this is a comment")
	 *    ;
	 */
	class CFG_API CfgCreator
	{
	public:
		CfgCreator();

		CfgCreator& configParseTextWithQuotes(bool enable);
		CfgCreator& configParseTextWithQuotesForName(bool enable);
		CfgCreator& configParseTextWithQuotesForValue(bool enable);

		// for empty line
		CfgCreator& empty();
		CfgCreator& comment(const std::string& comment, bool withSpace = true);

		CfgCreator& pushObject(const std::string& name);
		CfgCreator& popObject();

		CfgCreator& pushArray(const std::string& name);
		CfgCreator& popArray();

		// nvp ... name value pair
		CfgCreator& nvpNull(const std::string& name);
		CfgCreator& nvpBool(const std::string& name, bool value);
		CfgCreator& nvpFloat(const std::string& name, float value);
		CfgCreator& nvpInt(const std::string& name, int value);
		CfgCreator& nvpText(const std::string& name, const std::string& value);

		// val ... value
		CfgCreator& valNull();
		CfgCreator& valBool(bool value);
		CfgCreator& valFloat(float value);
		CfgCreator& valInt(int value);
		CfgCreator& valText(const std::string& value);

		// assign can be used to create name value pairs where the name has a non text type.
		// e.g. valInt(10).assign().valInt(7)
		CfgCreator& assign();

		Value getCfg() const;
		bool warningsExist() const { return !mWarnings.empty(); }
		std::string getWarningsAsString() const;
	private:
		bool isParseTextWithQuotesForName = false;
		bool isParseTextWithQuotesForValue = false;

		Value mCfg;
		unsigned int mDeep = 0;
		bool mAssign = false;
		std::vector<std::string> mWarnings;

		cfg::Value& getCurrent();
		void addWarning(const std::string& warning);
		void checkForResetAssign();
	};
}

#endif

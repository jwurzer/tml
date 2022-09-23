#ifndef CFG_CFG_TEMPLATE_H
#define CFG_CFG_TEMPLATE_H

#include <cfg/export.h>
#include <cfg/cfg.h>
#include <map>

namespace cfg
{
	class CFG_API CfgTemplate
	{
	public:
		CfgTemplate(const std::string& name,
				const std::vector<std::string>& parameters,
				const std::vector<NameValuePair>::const_iterator& begin,
				const std::vector<NameValuePair>::const_iterator& end);

		const std::vector<NameValuePair>& getPairs() const { return mObject; }
		std::size_t getParameterCount() const { return mParameters.size(); }
		const std::vector<std::string>& getParameters() const { return mParameters; }
		std::string toString() const;
	private:
		std::string mName;
		std::vector<std::string> mParameters;
		std::vector<NameValuePair> mObject;
	};

	namespace cfgtemp
	{
		typedef std::map<std::string, CfgTemplate> TemplateMap;

		CFG_API
		bool addTemplates(TemplateMap& templateMap, const std::string& tmlFilename,
				bool inclEmptyLines = false, bool inclComments = false,
				const std::string& templateKeyword = "template");

		/**
		 * Check the value tree (cfgValue) for templates and add all founded
		 * templates to the templateMap.
		 * @return Return true for success. False for a wrong template (wrong syntax).
		 *         If no template was found then also true is returned.
		 */
		CFG_API
		bool addTemplates(TemplateMap& templateMap, Value& cfgValue,
				bool removeTemplatesFromCfgValue,
				const std::string& templateKeyword = "template");

		/**
		 * Check the value tree (cfgValue) for templates and add all founded
		 * templates to the templateMap.
		 * @return Return true for success. False for a wrong template (wrong syntax).
		 *         If no template was found then also true is returned.
		 */
		CFG_API
		bool addTemplates(TemplateMap& templateMap, Value& cfgValue,
				bool removeTemplatesFromCfgValue,
				const std::string& templateKeyword,
				std::string& outErrorMsg);

		/**
		 * Replace the "use-template" references with the referenced templates.
		 * @return Return false if a syntax error happend
		 *         (wrong referenced template, wrong parameter count, ...).
		 *         True for success. True is also returned if no template
		 *         was replaced (no "use-template" reference was used).
		 */
		CFG_API
		bool useTemplates(const TemplateMap& templateMap, Value& cfgValue,
				const std::string& useTemplateKeyword = "use-template");

		/**
		 * Replace the "use-template" references with the referenced templates.
		 * @param checkForInterpreterExpressions Should only be set to true
		 *        if after template replacement also the interpreter is used.
		 *        useTemplates never uses the interpreter. If true it only
		 *        group the expressions for the parameters. This means if
		 *        true then the parameters can have interpreter expression
		 *        which can start with '_i (',  '_ii (',  '_fi ('  or  '_ti ('.
		 * @param allowInterpretationWithQuotes Only used if
		 *        checkForInterpreterExpressions is true. Otherwise ignored.
		 *        In this case (ignored) simply set it to false.
		 *        If source is parsed from a JSON file then this should be true.
		 *        If source is parsed from a TML file then this should be false.
		 *        For TML true would also work but then something like this
		 *        is interpreted: "abs" "(" -123 ")"  -->  123
		 *        If false then this would be unchanged.
		 *        If false then the example for interpretation must look like this:
		 *        abs ( -123 )  -->  123
		 * @return Return false if a syntax error happend
		 *         (wrong referenced template, wrong parameter count, ...).
		 *         True for success. True is also returned if no template
		 *         was replaced (no "use-template" reference was used).
		 */
		CFG_API
		bool useTemplates(const TemplateMap& templateMap, Value& cfgValue,
				const std::string& useTemplateKeyword,
				bool checkForInterpreterExpressions,
				bool allowInterpretationWithQuotes,
				std::string& outErrorMsg);
	}
}

#endif

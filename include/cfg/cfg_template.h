#ifndef CFG_CFG_TEMPLATE_H
#define CFG_CFG_TEMPLATE_H

#include <cfg/cfg.h>
#include <map>

namespace cfg
{
	class CfgTemplate
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

		/**
		 * Check the value tree (cfgValue) for templates and add all founded
		 * templates to the templateMap.
		 * @return Return true for success. False for a wrong template (wrong syntax).
		 *         If no template was found then also true is returned.
		 */
		bool addTemplates(TemplateMap& templateMap, Value& cfgValue,
				bool removeTemplatesFromCfgValue,
				const std::string& templateKeyword = "template");

		/**
		 * Check the value tree (cfgValue) for templates and add all founded
		 * templates to the templateMap.
		 * @return Return true for success. False for a wrong template (wrong syntax).
		 *         If no template was found then also true is returned.
		 */
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
		bool useTemplates(const TemplateMap& templateMap, Value& cfgValue,
				const std::string& useTemplateKeyword = "use-template");

		/**
		 * Replace the "use-template" references with the referenced templates.
		 * @return Return false if a syntax error happend
		 *         (wrong referenced template, wrong parameter count, ...).
		 *         True for success. True is also returned if no template
		 *         was replaced (no "use-template" reference was used).
		 */
		bool useTemplates(const TemplateMap& templateMap, Value& cfgValue,
				const std::string& useTemplateKeyword, std::string& outErrorMsg);
	}
}

#endif

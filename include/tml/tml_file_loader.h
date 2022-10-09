#ifndef CFG_TML_FILE_LOADER_H
#define CFG_TML_FILE_LOADER_H

#include <cfg/file_loader.h>
#include <tml/tml_parser.h>
#include <vector>

namespace cfg
{
	class CFG_API TmlFileLoader: public FileLoader
	{
	public:
		virtual std::string getFullFilename(const std::string& includeFilename) const override;
		virtual bool loadAndPush(Value& outValue, std::string& outFullFilename,
				const std::string& includeFilename,
				bool inclEmptyLines, bool inclComments,
				std::string& outErrorMsg) override;
		virtual bool pop() override;
		virtual unsigned int getNestedDeep() const override { return static_cast<unsigned int>(mPathStack.size()); }
	private:
		TmlParser mParser;
		std::vector<std::string> mPathStack;

		void push(const std::string& includeFilename);
		std::string getCurrentDir() const;
	};
}

#endif

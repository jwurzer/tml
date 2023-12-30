#ifndef CFG_PARSER_FILE_LOADER_H
#define CFG_PARSER_FILE_LOADER_H

#include <cfg/file_loader.h>
#include <cfg/value_parser.h>
#include <tml/tml_parser.h>
#include <vector>
#include <map>

namespace cfg
{
	/**
	 * Implementation of a FileLoader using internal a ValueParser to parse the file.
	 */
	class CFG_API ParserFileLoader: public FileLoader
	{
	public:
		ParserFileLoader(std::unique_ptr<ValueParser> parser);
		virtual ~ParserFileLoader() = default;
		virtual void reset() override;
		virtual std::string getFullFilename(const std::string& includeFilename) const override;
		virtual bool loadAndPush(Value& outValue, std::string& outFullFilename,
				const std::string& includeFilename,
				bool inclEmptyLines, bool inclComments,
				std::string& outErrorMsg) override;
		virtual bool pop() override;
		virtual unsigned int getNestedDeep() const override { return static_cast<unsigned int>(mPathStack.size()); }

		void setBuffering(bool buffering) { mBuffering = buffering; }
		void clearBufferedFiles() { mBufferedFiles.clear(); }
		void clearAndResetBuffering() { mBufferedFiles.clear(); mBuffering = false; }
	private:
		typedef std::map<std::string, Value> TFileBufferMap;

		std::unique_ptr<ValueParser> mParser;
		std::vector<std::string> mPathStack;

		bool mBuffering = false;
		TFileBufferMap mBufferedFiles;

		void push(const std::string& includeFilename);
		std::string getCurrentDir() const;
	};
}

#endif

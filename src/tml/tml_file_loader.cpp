#include <tml/tml_file_loader.h>

namespace cfg
{
	namespace
	{
		bool getDirnameWithDelim(const std::string& filename,
				std::string& dirname, char delim)
		{
			std::size_t found = filename.rfind(delim);
			if (found == std::string::npos) {
				return false;
			}

			dirname = filename.substr(0, found + 1);
			return true;
		}

		std::string getDirname(const std::string& filename)
		{
			std::string dirname;
			if (getDirnameWithDelim(filename, dirname, '\\')) {
				return dirname;
			}
			if (getDirnameWithDelim(filename, dirname, '/')) {
				return dirname;
			}
			return dirname;
		}
	}
}

bool cfg::TmlFileLoader::loadAndPush(Value& outValue, const std::string& filename,
		bool inclEmptyLines, bool inclComments, std::string& outErrorMsg)
{
	mParser.setFilename(getCurrentDir() + filename);
	if (!mParser.getAsTree(outValue, inclEmptyLines, inclComments)) {
		outErrorMsg = mParser.getExtendedErrorMsg();
		// must happend after reading err msg with getExtendedErrorMsg()
		// otherwise the error message is empty
		mParser.reset();
		outValue.clear();
		return false;
	}
	mParser.reset();
	push(filename);
	return true;
}

bool cfg::TmlFileLoader::pop()
{
	if (mPathStack.empty()) {
		return false;
	}
	mPathStack.pop_back();
	return true;
}

void cfg::TmlFileLoader::push(const std::string& filename)
{
	std::string fullFilename = getCurrentDir() + filename;
	mPathStack.push_back(getDirname(fullFilename));
}

std::string cfg::TmlFileLoader::getCurrentDir() const
{
	if (mPathStack.empty()) {
		return "";
	}
	return mPathStack.back();
}

#include <cfg/parser_file_loader.h>
#include <iostream>

namespace cfg
{
	namespace
	{
		/**
		 * Remove all ending slashes but never remove a slash at the beginning.
		 * Which means, if the first character is a '/' then it will not be removed.
		 * See special cases.
		 * And if more then one slashes to the next directory then these slashes
		 * will be reduced to one slash
		 *
		 * name           return
		 * "."            "."
		 * "/foo"         "/foo"
		 * "foo/"         "foo"
		 * "foo///"       "foo"
		 * "///foo//a//"  "/foo/a" - This result is different to getRemoveEndingSlashes()
		 * "/"            "/"      - special case!!!
		 * "/////"        "/"      - special case!!!
		 * ""             ""
		 */
		std::string reduceSlashesAndRemoveEndingSlashes(const std::string& name, char slash)
		{
			std::string tmp = name;
			size_t len = name.length();
			size_t ii = 0, oi = 0; // ii ... input index, oi ... output index
			bool prevWasCh = false;
			bool fullStringWithCh = true;
			for (; ii < len; ++ii) {
				if (name[ii] != slash) {
					if (prevWasCh) {
						tmp[oi] = slash;
						++oi;
					}
					tmp[oi] = name[ii];
					++oi;
					prevWasCh = false;
					fullStringWithCh = false;
				}
				else {
					prevWasCh = true;
				}
			}
			// also works for an empty string!
			return fullStringWithCh ? name.substr(0, 1) : tmp.substr(0, oi);
		}

		std::string reduceSlashesAndRemoveEndingSlashes(const std::string& name)
		{
			std::string tmp = reduceSlashesAndRemoveEndingSlashes(name, '/');
			return reduceSlashesAndRemoveEndingSlashes(tmp, '\\');
		}

		bool getDirname(const std::string& filename, std::string& dirname)
		{
			std::size_t found = filename.find_last_of("/\\");
			if (found == std::string::npos) {
				return false;
			}

			dirname = filename.substr(0, found + 1);
			return true;
		}

		std::string getDirname(const std::string& filename)
		{
			std::string dirname;
			if (getDirname(filename, dirname)) {
				return dirname;
			}
			return dirname;
		}
	}
}

cfg::ParserFileLoader::ParserFileLoader(std::unique_ptr<ValueParser> parser)
	:mParser(std::move(parser))
{
}

void cfg::ParserFileLoader::reset()
{
	mParser->reset();
	mPathStack.clear();
}

std::string cfg::ParserFileLoader::getFullFilename(const std::string& includeFilename) const
{
	std::string incFilename = includeFilename;
	std::string curDir = getCurrentDir();
	incFilename = reduceSlashesAndRemoveEndingSlashes(incFilename);
	bool modified = false;
	do {
		modified = false;
		if ((incFilename.compare(0, 3, "../") == 0 ||
				incFilename.compare(0, 3, "..\\") == 0) && curDir.size() >= 2) {
			curDir = getDirname(curDir.substr(0, curDir.size() - 1));
			incFilename = incFilename.substr(3);
			modified = true;
		}
		if (incFilename.compare(0, 2, "./") == 0 ||
				incFilename.compare(0, 2, ".\\") == 0) {
			incFilename = incFilename.substr(2);
			modified = true;
		}
	} while (modified);
	return curDir + incFilename;
}

bool cfg::ParserFileLoader::loadAndPush(Value& outValue, std::string& outFullFilename,
		const std::string& includeFilename, bool inclEmptyLines,
		bool inclComments, std::string& outErrorMsg)
{
	if (includeFilename.empty()) {
		outErrorMsg = "Empty filename is not allowed";
		outValue.clear();
		return false;
	}
	if (includeFilename.back() == '/' || includeFilename.back() == '\\') {
		outErrorMsg = "Filename " + includeFilename + " with an ending " +
				std::to_string(includeFilename.back()) + " is not allowed.";
		outValue.clear();
		return false;
	}
	outFullFilename = getFullFilename(includeFilename);
	bool loadWithParser = true;
	std::string filenameKey;
	if (mBuffering) {
		char postFix[] = "::__";
		postFix[2] = inclEmptyLines ? '1' : '0';
		postFix[3] = inclComments ? '1' : '0';
		filenameKey = outFullFilename + postFix;
		TFileBufferMap::iterator it = mBufferedFiles.find(filenameKey);
		if (it != mBufferedFiles.end()) {
			// --> found a already buffered version!
			outValue = it->second; // create a copy (no move etc.)
			loadWithParser = false;
		}
	}
	if (loadWithParser) {
		mParser->setFilename(outFullFilename);
		if (!mParser->getAsTree(outValue, inclEmptyLines, inclComments)) {
			outErrorMsg = mParser->getExtendedErrorMsg();
			// must happend after reading err msg with getExtendedErrorMsg()
			// otherwise the error message is empty
			mParser->reset();
			outValue.clear();
			return false;
		}
		mParser->reset();
		if (mBuffering) {
			// buffering is active --> filenameKey has already correct key (not empty)!
			mBufferedFiles[filenameKey] = outValue; // create a copy (no move etc.)
		}
	}
	push(includeFilename);
	return true;
}

bool cfg::ParserFileLoader::pop()
{
	if (mPathStack.empty()) {
		return false;
	}
	mPathStack.pop_back();
	return true;
}

void cfg::ParserFileLoader::push(const std::string& includeFilename)
{
	std::string fullFilename = getFullFilename(includeFilename);
	mPathStack.push_back(getDirname(fullFilename));
}

std::string cfg::ParserFileLoader::getCurrentDir() const
{
	if (mPathStack.empty()) {
		return "";
	}
	return mPathStack.back();
}

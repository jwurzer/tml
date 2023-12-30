#include <btml/btml_parser.h>
#include <btml/btml_stream.h>
#include <fstream>

cfg::BtmlParser::BtmlParser()
{
}

cfg::BtmlParser::BtmlParser(const std::string& filename)
{
	setFilename(filename);
}

cfg::BtmlParser::~BtmlParser()
{
}

void cfg::BtmlParser::reset()
{
	mFilename.clear();
	mData.clear();
	mDataIsValid = false;
	mErrorMsg.clear();
}

bool cfg::BtmlParser::setFilename(const std::string& filename)
{
	reset();
	mFilename = filename;
	return loadDataFromFile();
}

// inclEmptyLines and inclComments parameter are ignored!
bool cfg::BtmlParser::getAsTree(Value& root,
		bool /*inclEmptyLines*/, bool /*inclComments*/)
{
	if (!mDataIsValid) {
		if (mErrorMsg.empty()) {
			mErrorMsg = "No data available";
		}
		return false;
	}

	bool headerExist = false;
	bool stringTableExist = false;
	unsigned int stringTableEntryCount = 0;
	unsigned int stringTableSize = 0;

	unsigned int bytes = cfg::btmlstream::streamToValueWithOptionalHeader(
			mData.data(), static_cast<unsigned int>(mData.size()), root,
			&mErrorMsg, headerExist, stringTableExist, stringTableEntryCount,
			stringTableSize);
	if (bytes != mData.size()) {
		mErrorMsg += "warning: Convert btml to cfg::Value don't use all bytes. (" +
				std::to_string(bytes) + " != " + std::to_string(mData.size()) + ")";
		// only a warning, not an error
		// --> NO return false here!
	}
	if (!bytes) {
		return false;
	}
	return true;
}

// return filename with linenumber and error message
std::string cfg::BtmlParser::getExtendedErrorMsg() const
{
	return mFilename + ":" + mErrorMsg;
}

bool cfg::BtmlParser::loadDataFromFile()
{
	mDataIsValid = false;
	std::ifstream ifs(mFilename, std::ios::in | std::ios::binary);
	if (!ifs.is_open() || ifs.fail()) {
		mErrorMsg = "Can't open " + mFilename;
		return false;
	}
	ifs.seekg(0, std::ios::end);
	std::size_t size = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	mData.resize(size);
	ifs.read(reinterpret_cast<char*>(mData.data()), size);
	if (ifs.fail()) {
		mErrorMsg = "Can't read full file content";
		return false;
	}
	mDataIsValid = true;
	return true;
}

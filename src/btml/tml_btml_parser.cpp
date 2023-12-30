#include <btml/tml_btml_parser.h>

namespace {
	constexpr int TML_PARSER = 1;
	constexpr int BTML_PARSER = 2;
}

cfg::TmlBtmlParser::TmlBtmlParser()
{
}

cfg::TmlBtmlParser::TmlBtmlParser(const std::string& filename)
{
	setFilename(filename);
}

cfg::TmlBtmlParser::~TmlBtmlParser()
{
}

void cfg::TmlBtmlParser::reset()
{
	mFilename.clear();
	mCurrentParserSelection = 0;
	mTmlParser.reset();
	mBtmlParser.reset();
	mErrorMsg.clear();
}

bool cfg::TmlBtmlParser::setFilename(const std::string& filename)
{
	reset();
	mFilename = filename;
	std::size_t pos = mFilename.find_last_of('.');
	if (pos == std::string::npos) {
		mErrorMsg = "no filename extension (found no '.')";
		return false;
	}
	std::string fileExtension = mFilename.substr(pos + 1);
	if (fileExtension.empty()) {
		mErrorMsg = "filename extension is empty";
		return false;
	}
	if (fileExtension == "tml") {
		mCurrentParserSelection = TML_PARSER;
		return mTmlParser.setFilename(mFilename);
	}
	if (fileExtension == "btml") {
		mCurrentParserSelection = BTML_PARSER;
		return mBtmlParser.setFilename(mFilename);
	}
	mErrorMsg = "file extension '" + fileExtension + "' is not supported";
	return false;
}

bool cfg::TmlBtmlParser::getAsTree(Value& root,
		bool inclEmptyLines, bool inclComments)
{
	switch (mCurrentParserSelection) {
		case TML_PARSER:
			return mTmlParser.getAsTree(root, inclEmptyLines, inclComments);
		case BTML_PARSER:
			return mBtmlParser.getAsTree(root, inclEmptyLines, inclComments);
	}

	// --> mCurrentParserSelection is not TML and not BTML. --> no parser selection
	mErrorMsg = "No parser selected";
	return false;
}

// return filename with linenumber and error message
std::string cfg::TmlBtmlParser::getExtendedErrorMsg() const
{
	switch (mCurrentParserSelection) {
		case TML_PARSER:
			return mTmlParser.getExtendedErrorMsg();
		case BTML_PARSER:
			return mBtmlParser.getExtendedErrorMsg();
	}
	// --> mCurrentParserSelection is not TML and not BTML. --> no parser selection
	return mFilename + ":" + mErrorMsg;
}

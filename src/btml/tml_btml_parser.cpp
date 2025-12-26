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
	mCurrentParserSelection = getParserSelectionForFilename(filename,
			mErrorMsg);
	switch (mCurrentParserSelection) {
		case TML_PARSER:
			if (!mTmlParser.setFilename(mFilename)) {
				mErrorMsg += mTmlParser.getErrorMsg();
				return false;
			}
			return true;
		case BTML_PARSER:
			if (!mBtmlParser.setFilename(mFilename)) {
				mErrorMsg += mBtmlParser.getExtendedErrorMsg();
				return false;
			}
			return true;
		default:
			break;
	}
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

int cfg::TmlBtmlParser::getParserSelectionForFilename(
		const std::string& filename, std::string& outErrorMsg)
{
	std::size_t pos = filename.find_last_of('.');
	if (pos == std::string::npos) {
		outErrorMsg = "no filename extension (found no '.')";
		return 0;
	}
	std::string fileExtension = filename.substr(pos + 1);
	if (fileExtension.empty()) {
		outErrorMsg = "filename extension is empty";
		return 0;
	}
	if (fileExtension == "tml") {
		return TML_PARSER;
	}
	if (fileExtension == "btml") {
		return BTML_PARSER;
	}
	outErrorMsg = "file extension '" + fileExtension + "' is not supported";
	return 0;
}

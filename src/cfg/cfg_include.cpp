#include <cfg/cfg_include.h>
#include <cfg/file_loader.h>

#define MAX_RECURSIVE_DEEP 50

namespace cfg
{
namespace inc
{
namespace
{
	void moveDeepNumber(NameValuePair& nvp, int relativeDeepDiff)
	{
		if (relativeDeepDiff <= 0) {
			// With a negative value it makes no sense for include logic.
			// A negative value can be if a deep with -1 is used which
			// means "not defined". Therefore instead of an "undefined" deep
			// a deep by counting should be used.
			return;
		}
		// Only change the deep if not -1. Because -1 means not defined.
		if (nvp.mDeep >= 0) {
			if (nvp.isEmpty() && nvp.mDeep == 0) {
				// A empty line without indenting is unchanged.
				// This is the only exception.
				// A comment with no indenting is no exception and will be moved.
			} else {
				nvp.mDeep += relativeDeepDiff;
			}
		}
		if (nvp.mName.isObject()) {
			for (auto& child : nvp.mName.mObject) {
				moveDeepNumber(child, relativeDeepDiff);
			}
		}
		if (nvp.mValue.isObject()) {
			for (auto& child : nvp.mValue.mObject) {
				moveDeepNumber(child, relativeDeepDiff);
			}
		}
	}

	void moveDeepNumber(Value& value, int relativeDeepDiff)
	{
		for (NameValuePair& nvp : value.mObject) {
			moveDeepNumber(nvp, relativeDeepDiff);
		}
	}

	bool includeFilesNoBuffering(Value& cfgValue, FileLoader& loader,
			const std::string& includeKeyword, bool includeOnce, bool inclEmptyLines,
			bool inclComments, std::string& outErrorMsg,
			TFileMap& currentIncludedFiles, int currentDeep)
	{
		if (!cfgValue.isObject()) {
			outErrorMsg = "is no object";
			return false;
		}
		std::vector<NameValuePair>& pairs = cfgValue.mObject;
		std::size_t cnt = pairs.size();
		for (std::size_t i = 0; i < cnt; ++i) {
			NameValuePair& nvp = pairs[i];
			if (nvp.mValue.isObject()) {
				// here currentDeep + 1 must be used because its a child
				if (!includeFilesNoBuffering(nvp.mValue, loader, includeKeyword, includeOnce, inclEmptyLines,
						inclComments, outErrorMsg, currentIncludedFiles, currentDeep + 1)) {
					// no outErrorMsg = nvp.mValue.getFilenameAndPosition() +
					//		": " + outErrorMsg; here because outErrorMsg has already
					// the correct filename included.
					return false;
				}
			}
			if (!nvp.mName.isArray() || nvp.mName.mArray.size() < 2 ||
					nvp.mName.mArray[0].mText != includeKeyword) {
				// --> is no include statment --> jump to next
				continue;
			}

			// --> is a include statement and nvp.mName.mArray[1].mText is
			// the filename from the include file.

			const std::string& includeFilename = nvp.mName.mArray[1].mText;
			std::string fullFilename = loader.getFullFilename(includeFilename);
			Value includeValue;
			if (!includeOnce || currentIncludedFiles[fullFilename] == 0) {
				std::string outFullFilename;
				unsigned int origPathDeep = loader.getNestedDeep();
				if (!loader.loadAndPush(includeValue, outFullFilename,
						includeFilename, inclEmptyLines, inclComments,
						outErrorMsg)) {
					outErrorMsg = nvp.mName.getFilenameAndPosition() +
							": " + outErrorMsg;
					return false;
				}
				if (outFullFilename != fullFilename) {
					outErrorMsg = ": file loader has an invalid state (wrong full filename: " +
							outFullFilename + " != " + fullFilename + ")";
					// pop() because of successful loadAndPush().
					// Although the file loader has already an invalid state.
					// --> Maybe would nothing change by its invalid state.
					loader.pop();
					return false;
				}
				++currentIncludedFiles[outFullFilename];

				if (loader.getNestedDeep() != origPathDeep + 1) {
					outErrorMsg = ": file loader has an invalid state (wrong nested deep).";
					// pop() because of successful loadAndPush().
					// Although the file loader has already an invalid state.
					// --> Maybe would nothing change by its invalid state.
					loader.pop();
					return false;
				}

				if (loader.getNestedDeep() > MAX_RECURSIVE_DEEP) {
					outErrorMsg = "Reach max deep for includes! Maybe a recursive loop. (deep " +
							std::to_string(loader.getNestedDeep()) + ")";
					loader.pop(); // very important also at error because this pop() is from successful loadAndPush()
					return false;
				}

				// If a deep is defined than this is used as relative deep diff.
				// If no deep is defined than the current deep of recursive calls
				// (child deep) is used.
				// Only moveDeepNumber(includeValue, nvp.mDeep); would be a problem
				// because if no deep is defined (nvp.mDeep is -1) then no moving happend.
				// Only moveDeepNumber(includeValue, currentDeep); would be an
				// alternative. The current implementation respects also the stored
				// deep in nvp.mDeep.
				moveDeepNumber(includeValue, nvp.mDeep >= 0 ? nvp.mDeep : currentDeep);

				// at includeFilesNoBuffering() call currentDeep and NOT currentDeep + 1 must be used.
				if (!includeFilesNoBuffering(includeValue, loader, includeKeyword, includeOnce, inclEmptyLines,
						inclComments, outErrorMsg, currentIncludedFiles, currentDeep)) {
					loader.pop(); // very important also at error because this pop() is from successful loadAndPush()
					outErrorMsg = nvp.mName.getFilenameAndPosition() +
							": " + outErrorMsg;
					return false;
				}

				if (!loader.pop()) {
					outErrorMsg = nvp.mName.getFilenameAndPosition() +
							": pop failed for file loader";
					return false;
				}
				if (loader.getNestedDeep() != origPathDeep) {
					outErrorMsg = nvp.mName.getFilenameAndPosition() +
							": file loader has an invalid state (wrong nested deep).";
					return false;
				}
			}
			else {
				// includeOnce is true AND include file is already included!
				// --> simple include a empty object
				includeValue.setObject();
				if (inclComments) {
					includeValue.mObject.emplace_back();
					includeValue.mObject.back().setComment(" " + includeFilename + " is already included");
				}
				else if (inclEmptyLines) {
					includeValue.mObject.emplace_back();
				}

				moveDeepNumber(includeValue, nvp.mDeep >= 0 ? nvp.mDeep : currentDeep);
			}

			// NOW here! Inside includeValue all includes are replaced by their contents
			// --> no "include statement" exist in includeValue because of
			// includeFilesNoBuffering() call after loading for new loaded file.

			if (!includeValue.isObject()) {
				// Should not be really possible. A loaded file should be
				// always an object.
				outErrorMsg = nvp.mName.getFilenameAndPosition() +
						": " + includeFilename + " is not loaded as object.";
				return false;
			}

			// Check special case if an include statment has child name-value pairs.
			// which is allowed. But only can apply if the last name value pair
			// of the included content has a empty value which can be used as object.
			if (nvp.mValue.isObject()) {
				if (includeValue.mObject.empty()) {
					outErrorMsg = nvp.mName.getFilenameAndPosition() +
							": " + includeFilename +
							" is as empty object. Can't add child object.";
					return false;
				}
				if (!includeValue.mObject.back().mValue.isEmpty()) {
					outErrorMsg = nvp.mName.getFilenameAndPosition() +
							": Last name-value-pair of " + includeFilename +
							" has no empty value. Can't add child object.";
					return false;
				}
				includeValue.mObject.back().mValue = std::move(nvp.mValue);
			}

			if (includeValue.mObject.empty()) {
				pairs.erase(pairs.begin() + i);
				--i;
				--cnt;
			}
			else if (includeValue.mObject.size() == 1) {
				nvp = std::move(includeValue.mObject[0]);
			}
			else {
				pairs[i] = std::move(includeValue.mObject[0]);
				pairs.insert(pairs.begin() + i + 1,
						std::make_move_iterator(includeValue.mObject.begin() + 1),
						std::make_move_iterator(includeValue.mObject.end()));
				std::size_t increaseCount = includeValue.mObject.size() - 1;
				i += increaseCount;
				cnt += increaseCount;
			}
		}
		return true;
	}

	typedef std::map<std::string, Value> TFileBufferMap;

	bool includeFilesWithFileBuffering(Value& cfgValue, FileLoader& loader,
			const std::string& includeKeyword, bool includeOnce, bool inclEmptyLines,
			bool inclComments, std::string& outErrorMsg,
			TFileMap& currentIncludedFiles,
			TFileBufferMap& currentIncludedFileBuffers,
			int currentDeep)
	{
		if (!cfgValue.isObject()) {
			outErrorMsg = "is no object";
			return false;
		}
		std::vector<NameValuePair>& pairs = cfgValue.mObject;
		std::size_t cnt = pairs.size();
		for (std::size_t i = 0; i < cnt; ++i) {
			NameValuePair& nvp = pairs[i];
			if (nvp.mValue.isObject()) {
				// here currentDeep + 1 must be used because its a child
				if (!includeFilesWithFileBuffering(nvp.mValue, loader,
						includeKeyword, includeOnce, inclEmptyLines, inclComments,
						outErrorMsg, currentIncludedFiles,
						currentIncludedFileBuffers, currentDeep + 1)) {
					// no outErrorMsg = nvp.mValue.getFilenameAndPosition() +
					//		": " + outErrorMsg; here because outErrorMsg has already
					// the correct filename included.
					return false;
				}
			}
			if (!nvp.mName.isArray() || nvp.mName.mArray.size() < 2 ||
					nvp.mName.mArray[0].mText != includeKeyword) {
				// --> is no include statment --> jump to next
				continue;
			}

			// --> is a include statement and nvp.mName.mArray[1].mText is
			// the filename from the include file.

			const std::string& includeFilename = nvp.mName.mArray[1].mText;
			std::string fullFilename = loader.getFullFilename(includeFilename);
			Value includeValue;
			if (!includeOnce || currentIncludedFiles[fullFilename] == 0) {
				TFileBufferMap::iterator it = currentIncludedFileBuffers.find(fullFilename);
				if (it != currentIncludedFileBuffers.end()) {
					// --> already buffered with all sub-includes
					includeValue = it->second; // create a copy (no move, no ref)
				}
				else {
					// not found --> file not already buffered
					// --> read file and include all "sub-includes"
					// If includeValue is an object then it also already includes all
					// sub-includes (if sub-includes exist).

					std::string outFullFilename;
					unsigned int origPathDeep = loader.getNestedDeep();
					if (!loader.loadAndPush(includeValue, outFullFilename,
							nvp.mName.mArray[1].mText, inclEmptyLines,
							inclComments,
							outErrorMsg)) {
						outErrorMsg = nvp.mName.getFilenameAndPosition() +
								": " + outErrorMsg;
						return false;
					}
					if (outFullFilename != fullFilename) {
						outErrorMsg = ": file loader has an invalid state (wrong full filename: " +
								outFullFilename + " != " + fullFilename + ")";
						// pop() because of successful loadAndPush().
						// Although the file loader has already an invalid state.
						// --> Maybe would nothing change by its invalid state.
						loader.pop();
						return false;
					}
					++currentIncludedFiles[fullFilename];

					if (loader.getNestedDeep() != origPathDeep + 1) {
						outErrorMsg = ": file loader has an invalid state (wrong nested deep).";
						// pop() because of successful loadAndPush().
						// Although the file loader has already an invalid state.
						// --> Maybe would nothing change by its invalid state.
						loader.pop();
						return false;
					}

					if (loader.getNestedDeep() > MAX_RECURSIVE_DEEP) {
						outErrorMsg =
								"Reach max deep for includes! Maybe a recursive loop. (deep " +
										std::to_string(
												loader.getNestedDeep()) +
										")";
						loader.pop(); // very important also at error because this pop() is from successful loadAndPush()
						return false;
					}

					// at includeFilesWithFileBuffering() call currentDeep and NOT currentDeep + 1 must be used.
					if (!includeFilesWithFileBuffering(includeValue, loader,
							includeKeyword, includeOnce, inclEmptyLines, inclComments,
							outErrorMsg, currentIncludedFiles,
							currentIncludedFileBuffers, currentDeep)) {
						loader.pop(); // very important also at error because this pop() is from successful loadAndPush()
						outErrorMsg = nvp.mName.getFilenameAndPosition() +
								": " + outErrorMsg;
						return false;
					}

					if (!loader.pop()) {
						outErrorMsg = nvp.mName.getFilenameAndPosition() +
								": pop failed for file loader";
						return false;
					}
					if (loader.getNestedDeep() != origPathDeep) {
						outErrorMsg = nvp.mName.getFilenameAndPosition() +
								": file loader has an invalid state (wrong nested deep).";
						return false;
					}
					// copy back to file buffers
					currentIncludedFileBuffers[fullFilename] = includeValue;
				}

				// If a deep is defined than this is used as relative deep diff.
				// If no deep is defined than the current deep of recursive calls
				// (child deep) is used.
				// Only moveDeepNumber(includeValue, nvp.mDeep); would be a problem
				// because if no deep is defined (nvp.mDeep is -1) then no moving happend.
				// Only moveDeepNumber(includeValue, currentDeep); would be an
				// alternative. The current implementation respects also the stored
				// deep in nvp.mDeep.
				// moveDeepNumber must be happened after storing into file buffer
				// because multiple includes can have different indention.
				// The version without buffering can already call this function
				// after file loading before the recursive call for children.
				moveDeepNumber(includeValue, nvp.mDeep >= 0 ? nvp.mDeep : currentDeep);
			}
			else {
				// includeOnce is true AND include file is already included!
				// --> simple include a empty object
				includeValue.setObject();
				if (inclComments) {
					includeValue.mObject.emplace_back();
					includeValue.mObject.back().setComment(" " + includeFilename + " is already included");
				}
				else if (inclEmptyLines) {
					includeValue.mObject.emplace_back();
				}

				moveDeepNumber(includeValue, nvp.mDeep >= 0 ? nvp.mDeep : currentDeep);
			}

			// NOW here! Inside includeValue all includes are replaced by their contents
			// --> no "include statement" exist in includeValue because of
			// includeFilesWithFileBuffering() call after loading for new loaded file.

			if (!includeValue.isObject()) {
				// Should not be really possible. A loaded file should be
				// always an object.
				outErrorMsg = nvp.mName.getFilenameAndPosition() +
						": " + includeFilename + " is not loaded as object.";
				return false;
			}

			// Check special case if an include statment has child name-value pairs.
			// which is allowed. But only can apply if the last name value pair
			// of the included content has a empty value which can be used as object.
			if (nvp.mValue.isObject()) {
				if (includeValue.mObject.empty()) {
					outErrorMsg = nvp.mName.getFilenameAndPosition() +
							": " + includeFilename +
							" is as empty object. Can't add child object.";
					return false;
				}
				if (!includeValue.mObject.back().mValue.isEmpty()) {
					outErrorMsg = nvp.mName.getFilenameAndPosition() +
							": Last name-value-pair of " + includeFilename +
							" has no empty value. Can't add child object.";
					return false;
				}
				includeValue.mObject.back().mValue = std::move(nvp.mValue);
			}

			if (includeValue.mObject.empty()) {
				pairs.erase(pairs.begin() + i);
				--i;
				--cnt;
			} else if (includeValue.mObject.size() == 1) {
				nvp = std::move(includeValue.mObject[0]);
			} else {
				pairs[i] = std::move(includeValue.mObject[0]);
				pairs.insert(pairs.begin() + i + 1,
						std::make_move_iterator(includeValue.mObject.begin() + 1),
						std::make_move_iterator(includeValue.mObject.end()));
				std::size_t increaseCount =
						includeValue.mObject.size() - 1;
				i += increaseCount;
				cnt += increaseCount;
			}
		}
		return true;
	}
} // namespace {}
} // namespace inc
} // namespace cfg

bool cfg::inc::loadAndIncludeFiles(Value& outValue, TFileMap& outIncludedFiles,
		const std::string& filename, FileLoader& loader,
		const std::string& includeKeyword, bool includeOnce,
		bool inclEmptyLines, bool inclComments, bool withFileBuffering,
		std::string& outErrorMsg)
{
	unsigned int origPathDeep = loader.getNestedDeep();
	std::string outFullFilename;
	if (!loader.loadAndPush(outValue, outFullFilename, filename,
			inclEmptyLines, inclComments, outErrorMsg)) {
		outErrorMsg += ": load " + filename + " failed";
		outValue.clear();
		return false;
	}
	if (loader.getNestedDeep() != origPathDeep + 1) {
		outErrorMsg = ": file loader has an invalid state.";
		// pop() because of successful loadAndPush().
		// Although the file loader has already an invalid state.
		// --> Maybe would nothing change by its invalid state.
		loader.pop();
		outValue.clear();
		return false;
	}
	bool rv = includeFiles(outValue, loader, includeKeyword, includeOnce,
			inclEmptyLines, inclComments, withFileBuffering, outErrorMsg,
			outIncludedFiles, 0);
	if (!rv) {
		outValue.clear();
	}
	if (!loader.pop() || loader.getNestedDeep() != origPathDeep) {
		if (!rv) {
			outErrorMsg += ": file loader has an invalid state.";
		}
		else {
			outErrorMsg = ": file loader has an invalid state.";
		}
		outValue.clear();
		return false;
	}
	return rv;
}

bool cfg::inc::includeFiles(Value& cfgValue, FileLoader& loader,
		const std::string& includeKeyword, bool includeOnce,
		bool inclEmptyLines, bool inclComments,
		bool withFileBuffering, std::string& outErrorMsg)
{
	TFileMap currentIncludedFiles;
	return includeFiles(cfgValue, loader, includeKeyword, includeOnce,
			inclEmptyLines, inclComments, withFileBuffering, outErrorMsg,
			currentIncludedFiles, 0);
}

bool cfg::inc::includeFiles(Value& cfgValue, FileLoader& loader,
		const std::string& includeKeyword, bool includeOnce,
		bool inclEmptyLines, bool inclComments,
		bool withFileBuffering, std::string& outErrorMsg,
		TFileMap& currentIncludedFiles, int currentDeep)
{
	if (withFileBuffering) {
		TFileBufferMap currentIncludedFileBuffers;

		return includeFilesWithFileBuffering(cfgValue, loader,
				includeKeyword, includeOnce, inclEmptyLines, inclComments, outErrorMsg,
				currentIncludedFiles, currentIncludedFileBuffers,
				currentDeep);
	}
	else {
		return includeFilesNoBuffering(cfgValue, loader,
				includeKeyword, includeOnce, inclEmptyLines, inclComments, outErrorMsg,
				currentIncludedFiles, currentDeep);
	}
}

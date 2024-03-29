#include <cfg/cfg_template.h>
#include <tml/tml_string.h>
#include <tml/tml_parser.h>
#include <sstream>

#define MAX_RECURSIVE_DEEP 50

namespace cfg
{
	namespace cfgtemp
	{
		namespace
		{
			typedef std::map<std::string, Value> ParameterMap;

			/**
			 * Modify the deep number of the "value tree".
			 * This function is necessary because if a "use-template" reference
			 * has another deep as the content of the template then
			 * the deep numbers of the copied template content must be changed
			 * to the correct deeps.
			 */
			void replaceDeepNumber(NameValuePair& nvp, int absDeep,
					int relativeDeepDiff, bool usingRelativeDeepDiff) {
				if (nvp.mDeep >= 0) {
					if (usingRelativeDeepDiff) {
						nvp.mDeep += relativeDeepDiff;
					}
					else {
						nvp.mDeep = absDeep;
					}
				}
				if (nvp.mName.isObject()) {
					for (auto& child : nvp.mName.mObject) {
						replaceDeepNumber(child, absDeep + 1, relativeDeepDiff,
								usingRelativeDeepDiff);
					}
				}
				if (nvp.mValue.isObject()) {
					for (auto& child : nvp.mValue.mObject) {
						replaceDeepNumber(child, absDeep + 1, relativeDeepDiff,
								usingRelativeDeepDiff);
					}
				}
			}

			/**
			 * Replace the used parameters by there arguments
			 */
			void replaceValueByParameters(const ParameterMap& parameterMap,
					Value& cfgValue) {
				if (cfgValue.isText()) {
					ParameterMap::const_iterator it = parameterMap.find(cfgValue.mText);
					if (it == parameterMap.end()) {
						return;
					}
					cfgValue = it->second;
					return;
				}
				if (cfgValue.isArray()) {
					std::size_t cnt = cfgValue.mArray.size();
					for (std::size_t i = 0; i < cnt; ++i) {
						replaceValueByParameters(parameterMap, cfgValue.mArray[i]);
					}
					return;
				}
				if (cfgValue.isObject()) {
					std::vector<NameValuePair>& pairs = cfgValue.mObject;
					std::size_t cnt = pairs.size();
					for (std::size_t i = 0; i < cnt; ++i) {
						replaceValueByParameters(parameterMap, cfgValue.mObject[i].mName);
						replaceValueByParameters(parameterMap, cfgValue.mObject[i].mValue);
					}
					return;
				}
			}

			/**
			 * Replace the used parameters by there arguments
			 */
			void replaceValueByParameters(const ParameterMap& parameterMap,
					NameValuePair& nameValuePair) {
				replaceValueByParameters(parameterMap, nameValuePair.mName);
				replaceValueByParameters(parameterMap, nameValuePair.mValue);
			}

			bool isUsingTemplate(const Value& cfgValue,
					const std::string& keywordForUsingTemplate)
			{
				return cfgValue.isArray() && !cfgValue.mArray.empty() &&
						cfgValue.mArray[0].mText == keywordForUsingTemplate;
			}

			/**
			 * @note Some parts of this function are very simular to
			 * cfg::interpreter::interpretAndReplaceExprValue().
			 * @param tmpResult Only filled if return value is > 0.
			 * @return -1 for error. 0 for no expressions. >0 for expression count
			 */
			int getParameterValues(const std::vector<Value>& array,
					cfg::Value& tmpResult, bool allowInterpretationWithQuotes,
					std::string& outErrMsg)
			{
				std::size_t count = array.size();
				if (count < 2) {
					return -1;
				}
				tmpResult.setArray();
				unsigned int expressionCount = 0;
				// nextStartIndex ... next start for an expression
				unsigned int nextStartIndex = 2;
				for (unsigned int i = 3; i < count; ++i) {
					if (i <= nextStartIndex) {
						// should not be possible
						continue;
					}
					// --> i > nextStartIndex --> i can only be >= 3
					const cfg::Value& val = array[i];
					if (!val.isText() || val.mText != "(" ||
							(!allowInterpretationWithQuotes && val.mParseTextWithQuotes)) {
						continue;
					}
					const cfg::Value& prev = array[i - 1];
					if (!prev.isText()) {
						continue;
					}
					if (prev.mText != "_i" && prev.mText != "_ii" &&
							prev.mText != "_fi" && prev.mText != "_ti") {
						continue;
					}
					if (!allowInterpretationWithQuotes && prev.mParseTextWithQuotes) {
						continue;
					}
					// --> expression for interpreter start with
					// '_i (',  '_ii (',  '_fi ('  or  '_ti ('
					++expressionCount;
					unsigned int curStartIndex = i - 1;

					// ci ... copy index
					for (unsigned int ci = nextStartIndex; ci < curStartIndex; ++ci) {
						// without std::move() because we doesn't want change the original input.
						// Because if an error happened parameter array should be unchanged.
						//std::cout << "copy " << ci << std::endl;
						tmpResult.mArray.push_back(array[ci]);
					}

					int parenCount = 1;
					for (++i; i < count && parenCount > 0; ++i) {
						const cfg::Value& next = array[i];
						if (next.isText() && !next.mParseTextWithQuotes && next.mText.size() == 1) {
							char ch = next.mText[0];
							if (ch == '(') {
								++parenCount;
							}
							else if (ch == ')') {
								--parenCount;
							}
						}
					}
					if (parenCount > 0) {
						outErrMsg = "Can't find ending";
						return -1;
					}

					nextStartIndex = i;

					// other loop make ++i --> --i would "normally" be necessary here.
					// But not necessary because start sequence for interpreter takes
					// to token like '_i (',  '_ii (',  '_fi ('  or  '_ti ('
					// not necessary: --i;

					Value expr;
					expr.setArray();

					// ci ... copy index
					for (unsigned int ci = curStartIndex; ci < nextStartIndex; ++ci) {
						expr.mArray.push_back(array[ci]);
					}
					tmpResult.mArray.push_back(expr);
				}

				if (expressionCount > 0) {
					for (unsigned int ci = nextStartIndex; ci < count; ++ci) {
						// also here no std::move()
						//std::cout << "finish copy " << ci << std::endl;
						tmpResult.mArray.push_back(array[ci]);
					}
					return expressionCount;
				}
				return 0;
			}

			/**
			 * Get the template
			 * @param templateMap Map of all templates
			 * @param cfgValue Value which includes the "use-template" reference
			 * @param parameterMap Stores the arguments for the parameter.
			 *        This map is filled by this function. All existing entries
			 *        are discard.
			 * @param outErrorMsg Error message. In this case -1 is returned.
			 * @return The pointer to the valid template or null for an error.
			 *         E.g. null is also returned if the count of parameters
			 *         are wrong.
			 */
			const CfgTemplate* getTemplate(const TemplateMap& templateMap,
					const Value& cfgValue,
					bool checkForInterpreterExpressions,
					bool allowInterpretationWithQuotes,
					ParameterMap& parameterMap,
					std::string& outErrorMsg)
			{
				if (!cfgValue.isArray()) {
					outErrorMsg = cfgValue.getFilenameAndPosition() + ": is not an array";
					return nullptr;
				}
				std::size_t cnt = cfgValue.mArray.size();
				if (cnt < 2) {
					outErrorMsg = cfgValue.getFilenameAndPosition() + ": empty array is not allowed";
					return nullptr;
				}
				TemplateMap::const_iterator it = templateMap.find(
						cfgValue.mArray[1].mText);
				if (it == templateMap.end()) {
					outErrorMsg = cfgValue.mArray[1].getFilenameAndPosition() +
							": template '" + cfgValue.mArray[1].mText +
							"' not found";
					return nullptr;
				}
				const Value* paramValues = cfgValue.mArray.data() + 2;
				std::size_t paramValueCount = cnt - 2;
				Value tmpResult;

				if (checkForInterpreterExpressions) {
					std::string exprErrMsg;
					int rv = getParameterValues(cfgValue.mArray,
							tmpResult, allowInterpretationWithQuotes,
							exprErrMsg);
					if (rv == -1) {
						outErrorMsg =
								cfgValue.mArray[1].getFilenameAndPosition() +
										": " + exprErrMsg;
						return nullptr;
					}
					if (rv > 0) {
						paramValues = tmpResult.mArray.data();
						paramValueCount = tmpResult.mArray.size();
					}
				}

				if (it->second.getParameterCount() != paramValueCount) {
					outErrorMsg = cfgValue.mArray[1].getFilenameAndPosition() +
							": wrong parameter count for template '" +
							cfgValue.mArray[1].mText + "' must be: " +
							std::to_string(it->second.getParameterCount()) +
							", is: " + std::to_string(paramValueCount);
					return nullptr;
				}
				parameterMap.clear();
				const std::vector<std::string>& params = it->second.getParameters();
				std::size_t paramCount = params.size();
				for (std::size_t i = 0; i < paramCount; ++i) {
					if (!parameterMap.insert(ParameterMap::value_type(
							params[i], paramValues[i])).second) {
						outErrorMsg = paramValues[i].getFilenameAndPosition() +
								": create parameter map failed";
						return nullptr;
					}
				}
				return &it->second;
			}

			bool applyTemplates(const TemplateMap& templateMap,
					std::vector<NameValuePair>& pairs,
					int pairStartIndex, int pairCount,
					const std::string& keywordForUsingTemplate,
					bool checkForInterpreterExpressions,
					bool allowInterpretationWithQuotes,
					int currentRecursiveReplaceDeep,
					std::vector<std::string>& templateNameStack,
					int& outPairAddRemoveCount, std::string& outErrorMsg);

			/**
			 * Replace a "use-template" reference by the referenced template.
			 * None, one or more pairs can be added by the replacing.
			 *
			 * The methode is recursive. Which means that a replaced template
			 * can have again a "use-template" reference which will be also
			 * replaced by the referenced template and so on.
			 *
			 * @param templateMap Map of all templates
			 * @param pairs pairs of the current object
			 * @param index Index of the "use-template" reference which will be replaced
			 * @param keywordForUsingTemplate The used keyword (usually "use-template")
			 * @param currentRecursiveReplaceDeep Is needed to limit to a max recursive deep
			 * @param templateNameStack Is needed to check recursive loops
			 * @param outErrorMsg Error message. In this case -1 is returned.
			 * @return The count of modified and added name-value-pairs.
			 *         Value of 0 means the pair pairs[index] is erased.
			 *         Value of 1 means the pair pairs[index] is replaced.
			 *         A Value of n means the pair pairs[index] is replaced
			 *         and the pairs pairs[index + 1] to pairs[index + n - 1] are
			 *         inserted. --> n - 1 are inserted.
			 *         Return -1 for an error.
			 */
			int replaceTemplate(const TemplateMap& templateMap,
					std::vector<NameValuePair>& pairs, std::size_t index,
					const std::string& keywordForUsingTemplate,
					bool checkForInterpreterExpressions,
					bool allowInterpretationWithQuotes,
					int currentRecursiveReplaceDeep,
					std::vector<std::string>& templateNameStack,
					std::string& outErrorMsg)
			{
				NameValuePair& nvp = pairs[index];
				ParameterMap parameterMap;
				const CfgTemplate* cfgTemp = getTemplate(templateMap,
						nvp.mName,
						checkForInterpreterExpressions,
						allowInterpretationWithQuotes,
						parameterMap, outErrorMsg);
				if (!cfgTemp) {
					return -1;
				}
				std::string tempName = nvp.mName.mArray[1].mText;
				for (const auto& name : templateNameStack) {
					if (name == tempName) {
						// found recursive loop
						outErrorMsg = "Recursive template loop: ";
						for (const auto& n : templateNameStack) {
							outErrorMsg += n + " --> ";
						}
						outErrorMsg += tempName;
						return -1;
					}
				}
				const CfgTemplate& cfgTemplate = *cfgTemp;
				const std::vector<NameValuePair>& tmpPairs = cfgTemplate.getPairs();
				if (tmpPairs.empty()) {
					pairs.erase(pairs.begin() + index);
					return 0;
				}
				int origDeep = pairs[index].mDeep;
				pairs[index] = tmpPairs[0];
				pairs.insert(pairs.begin() + index + 1, tmpPairs.begin() + 1,
						tmpPairs.end());
				std::size_t cnt = tmpPairs.size();
				for (std::size_t i = 0; i < cnt; ++i) {
					replaceValueByParameters(parameterMap, pairs[index + i]);
					if (origDeep >= 0) {
						int tempDeep = pairs[index + i].mDeep;
						replaceDeepNumber(pairs[index + i], origDeep,
								origDeep - tempDeep, tempDeep >= 0);
					}
				}
				int pairAddRemoveCount = 0;
				templateNameStack.push_back(tempName);
				if (!applyTemplates(templateMap, pairs, static_cast<int>(index), static_cast<int>(tmpPairs.size()),
						keywordForUsingTemplate,
						checkForInterpreterExpressions,
						allowInterpretationWithQuotes,
						currentRecursiveReplaceDeep + 1,
						templateNameStack,
						pairAddRemoveCount, outErrorMsg)) {
					templateNameStack.pop_back();
					return -1;
				}
				templateNameStack.pop_back();
				if (pairAddRemoveCount < -int(tmpPairs.size())) {
					// should not be possible
					outErrorMsg = "Recursive error";
					return -1;
				}
				return int(tmpPairs.size()) + pairAddRemoveCount;
			}

			/**
			 * Replace a "use-template" reference by the referenced template.
			 * Only a simple template is allowed which means that the
			 * replacement doesn't add or remove name-value-pairs.
			 *
			 * The methode is recursive. Which means that a replaced template
			 * can have again a "use-template" reference which will be also
			 * replaced by the referenced template and so on.
			 *
			 * @param templateMap Map of all templates
			 * @param cfgValue A name or value of a name-value pair.
			 * @param keywordForUsingTemplate The used keyword (usually "use-template")
			 * @param currentRecursiveReplaceDeep Is needed to limit to a max recursive deep
			 * @param templateNameStack Is needed to check recursive loops
			 * @param outErrorMsg Error message. In this case false is returned.
			 * @return Return true for success.
			 */
			bool replaceSimpleTemplate(const TemplateMap& templateMap, Value& cfgValue,
					const std::string& keywordForUsingTemplate,
					bool checkForInterpreterExpressions,
					bool allowInterpretationWithQuotes,
					int currentRecursiveReplaceDeep,
					std::vector<std::string>& templateNameStack,
					std::string& outErrorMsg) {
				if (currentRecursiveReplaceDeep > MAX_RECURSIVE_DEEP) {
					outErrorMsg = "Reach max recursive deep for simple template replacement! (deep " +
							std::to_string(currentRecursiveReplaceDeep) + ")";
					return false;
				}

				ParameterMap parameterMap;
				const CfgTemplate* cfgTemp = getTemplate(templateMap,
						cfgValue,
						checkForInterpreterExpressions,
						allowInterpretationWithQuotes,
						parameterMap, outErrorMsg);
				if (!cfgTemp) {
					return false;
				}
				std::string tempName = cfgValue.mArray[1].mText;
				for (const auto& name : templateNameStack) {
					if (name == tempName) {
						// found recursive loop
						outErrorMsg = "Recursive template loop (simple replacement): ";
						for (const auto& n : templateNameStack) {
							outErrorMsg += n + " --> ";
						}
						outErrorMsg += tempName;
						return false;
					}
				}
				const CfgTemplate& cfgTemplate = *cfgTemp;
				const std::vector<NameValuePair>& tmpPairs = cfgTemplate.getPairs();
				if (tmpPairs.size() < 1) {
					outErrorMsg = tempName + ": Empty template is not allowed";
					return false;
				}
				std::size_t cnt = tmpPairs.size();
				int index = -1;
				for (std::size_t i = 0; i < cnt; ++i) {
					if (tmpPairs[i].isEmptyOrComment()) {
						continue;
					}
					if (index != -1) {
						outErrorMsg = tempName +
								": Only a simple template is allowed (" +
								std::to_string(tmpPairs.size()) +
								", no multiple nv-pairs)";
						return false;
					}
					index = static_cast<int>(i);
				}
				if (index == -1) {
					outErrorMsg = tempName +
							": Only a simple template is allowed (" +
							std::to_string(tmpPairs.size()) +
							", no nv-pairs)";
					return false;
				}
				const NameValuePair& nvp = tmpPairs[index];
				if (nvp.isEmptyOrComment()) {
					outErrorMsg = tempName +
							": Only a simple template is allowed (no empty or comment)";
					return false;
				}
				if (!nvp.mValue.isEmpty()) {
					outErrorMsg = tempName +
							": Only a simple template is allowed (value must be empty)";
					return false;
				}
				if (nvp.mName.isObject()) {
					outErrorMsg = tempName +
							": Only a simple template is allowed (name can't be an object)";
					return false;
				}
				cfgValue = nvp.mName;
				replaceValueByParameters(parameterMap, cfgValue);
				if (isUsingTemplate(cfgValue, keywordForUsingTemplate)) {
					templateNameStack.push_back(tempName);
					if (!replaceSimpleTemplate(templateMap, cfgValue,
							keywordForUsingTemplate,
							checkForInterpreterExpressions,
							allowInterpretationWithQuotes,
							currentRecursiveReplaceDeep + 1,
							templateNameStack, outErrorMsg)) {
						templateNameStack.pop_back();
						return false;
					}
					templateNameStack.pop_back();
				}
				return true;
			}

			/**
			 * Replace all "use-template" references by there referenced templates.
			 *
			 * This method is recursive. Which means that a replaced template
			 * can have again a "use-template" reference which will be also
			 * replaced by the referenced template and so on.
			 *
			 * @param templateMap Map of all templates
			 * @param pairs pairs of the current object
			 * @param pairStartIndex start index of the first nv-pair which
			 *        should be used. Or -1 for default (which is 0).
			 * @param pairCount Count of used nv-pairs starting with pairStartIndex.
			 *        -1 for the max possible count (start from pairStartIndex).
			 * @param keywordForUsingTemplate The used keyword (usually "use-template")
			 * @param currentRecursiveReplaceDeep Is needed to limit to a max recursive deep.
			 *        This has nothing to do with the current deep of the
			 *        "value tree". This deep value is only for the recursive
			 *        replacement of templates.
			 * @param templateNameStack Is needed to check recursive loops
			 * @param outPairAddRemoveCount Count of added or removed pairs.
			 *        Positive for added count. Negative for removed count.
			 * @param outErrorMsg Error message. In this case false is returned.
			 * @return True for success. False for error.
			 */
			bool applyTemplates(const TemplateMap& templateMap,
					std::vector<NameValuePair>& pairs,
					int pairStartIndex, int pairCount,
					const std::string& keywordForUsingTemplate,
					bool checkForInterpreterExpressions,
					bool allowInterpretationWithQuotes,
					int currentRecursiveReplaceDeep,
					std::vector<std::string>& templateNameStack,
					int& outPairAddRemoveCount, std::string& outErrorMsg)
			{
				if (currentRecursiveReplaceDeep > MAX_RECURSIVE_DEEP) {
					outErrorMsg = "Reach max recursive deep for template replacement! (deep " +
							std::to_string(currentRecursiveReplaceDeep) + ")";
					return false;
				}
				outPairAddRemoveCount = 0;
				if (pairStartIndex < 0) {
					pairStartIndex = 0;
				}
				if (pairStartIndex >= int(pairs.size())) {
					// nothing to do
					return true;
				}
				if (pairCount < 0) {
					pairCount = int(pairs.size()) - pairStartIndex;
				}
				int limit = pairStartIndex + pairCount;
				for (int i = 0; i < limit; ++i) {
					bool isReplaced = false;
					if (i < 0) {
						outErrorMsg = "Apply templates: Out of range " + std::to_string(i) + " < 0. size: " + std::to_string(pairs.size());
						return false;
					}
					if (i >= int(pairs.size())) {
						outErrorMsg = "Apply templates: Out of range " + std::to_string(i) + " >= " + std::to_string(pairs.size());
						return false;
					}
					NameValuePair* nvp = &pairs[i];
					if (nvp->isObject()) {
						int pairDiffCount = 0;
						std::vector<std::string> tempStack;
						if (!applyTemplates(templateMap, nvp->mValue.mObject,
								-1, -1, keywordForUsingTemplate,
								checkForInterpreterExpressions,
								allowInterpretationWithQuotes,
								0, tempStack,
								pairDiffCount, outErrorMsg)) {
							return false;
						}
					}
					if (isUsingTemplate(nvp->mName, keywordForUsingTemplate)) {
						if (nvp->mValue.isEmpty() || nvp->mValue.isObject()) {
							cfg::Value origValue = std::move(nvp->mValue);
							// full replacment with none, one or more name-value-pairs are allowed
							int insertCnt = replaceTemplate(templateMap, pairs,
									i, keywordForUsingTemplate,
									checkForInterpreterExpressions,
									allowInterpretationWithQuotes,
									currentRecursiveReplaceDeep,
									templateNameStack,
									outErrorMsg);
							if (insertCnt < 0) {
								return false;
							}

							// --i --> jump one back --> repeat the template check at next iteration.
							// This is necessary because the used template can
							// also use a template ...
							//--i;
							i += insertCnt - 1; // this would be the opposite. omit all new added pairs
							// i can be -1 if insertCnt is 0 and i was 0 --> result: -1
							// this is no problem
							outPairAddRemoveCount += insertCnt - 1;
							isReplaced = true;
							limit += insertCnt - 1;

							// replaceTemplate() can change the pairs
							// --> pairs maybe make a reallocation for increasing
							// --> nvp is NOT valid any more
							// --> reassign nvp to current name value pair
							if (i >= int(pairs.size())) {
								outErrorMsg = "Apply templates: Out of range after replace " + std::to_string(i) + " >= " + std::to_string(pairs.size());
								return false;
							}
							// set to null if insertCnt is 0. &pairs[i] is not always valid because i can be -1 if insertCnt is 0!
							nvp = (insertCnt == 0) ? nullptr : &pairs[i]; // VERY IMPORTANT!!!!!

							if (origValue.isObject()) {
								if (insertCnt == 0) {
									outErrorMsg = "Can't add children after an empty template";
									return false;
								}
								int realInsertCnt = insertCnt;
								// find a valid name-value pair.
								// A empty name-value pair is not allowed therefore
								// search the last non-empty name-value pair.
								NameValuePair* validNvp = &pairs[i];
								for (int revert = 0;
										realInsertCnt >= 1 && validNvp->isEmpty();
										++revert, --realInsertCnt, validNvp = &pairs[i - revert])
									;
								if (realInsertCnt <= 0) {
									outErrorMsg = "Can't add children after an empty template";
									return false;
								}

								if (validNvp->isEmptyOrComment() ||
										validNvp->mName.isEmpty() || validNvp->mName.isComment()) {
									outErrorMsg = "Can't add children to an empty name or comment";
									return false;
								}
								if (!validNvp->mValue.isEmpty() && !validNvp->mValue.isObject()) {
									outErrorMsg = "Can't add children. The value from the template's last name value pair must be empty or an object.";
									return false;
								}
								if (!validNvp->isObject()) {
									// check is necessary because setObject also clear the current content
									// --> only if no object (or an object with no childrens/entries) then setObject() is ok
									validNvp->mValue.setObject();
								}
								validNvp->mValue.mObject.insert(validNvp->mValue.mObject.end(),
										std::make_move_iterator(origValue.mObject.begin()),
										std::make_move_iterator(origValue.mObject.end()));
							}
						}
						else {
							// only a simple replacement is allowed
							if (!replaceSimpleTemplate(templateMap, nvp->mName,
									keywordForUsingTemplate,
									checkForInterpreterExpressions,
									allowInterpretationWithQuotes,
									currentRecursiveReplaceDeep,
									templateNameStack, outErrorMsg)) {
								return false;
							}
							// --i --> jump one back --> repeat the template check at next iteration.
							// This is necessary because the used template can
							// also use a template ...
							//--i;
							isReplaced = true;
						}
					}
					// nvp can be null here if insertCnt of replaceTemplate was 0
					if (nvp && isUsingTemplate(nvp->mValue, keywordForUsingTemplate)) {
						// only a simple replacement is allowed
						if (!replaceSimpleTemplate(templateMap, nvp->mValue,
								keywordForUsingTemplate,
								checkForInterpreterExpressions,
								allowInterpretationWithQuotes,
								currentRecursiveReplaceDeep,
								templateNameStack, outErrorMsg)) {
							return false;
						}
						// check if i must be decreased for repeat the check.
						// if i is already decreased (--i) then this is not necessary (otherwise wrong i value)
						if (!isReplaced) {
							//--i;
						}
					}
				}
				return true;
			}
		}
	}
}

cfg::CfgTemplate::CfgTemplate(const std::string& name,
		const std::vector<std::string>& parameters)
		:mName(name), mParameters(parameters), mObject()
{
}

cfg::CfgTemplate::CfgTemplate(const std::string& name,
		const std::vector<std::string>& parameters,
		const std::vector<NameValuePair>::const_iterator& begin,
		const std::vector<NameValuePair>::const_iterator& end)
		:mName(name), mParameters(parameters), mObject(begin, end)
{
}

std::string cfg::CfgTemplate::toString() const
{
	std::stringstream ss;
	ss << "name: " << mName << "\n";
	ss << "parameters: " << mParameters.size();
	std::size_t cnt = mParameters.size();
	if (cnt) {
		ss << ":";
	}
	for (std::size_t i = 0; i < cnt; ++i) {
		ss << " " << mParameters[i];
	}
	ss << "\n";
	ss << "tml:\n";
	for (const auto& pair : mObject) {
		cfg::tmlstring::nameValuePairToStream(1, pair, ss, false);
	}
	return ss.str();
}

bool cfg::cfgtemp::addTemplates(TemplateMap& templateMap,
		const std::string& tmlFilename, bool inclEmptyLines, bool inclComments,
		const std::string& templateKeyword)
{
	TmlParser parser(tmlFilename);
	if (!parser.begin()) {
		return false;
	}
	Value root;
	if (!parser.getAsTree(root, inclEmptyLines, inclComments)) {
		return false;
	}
	return addTemplates(templateMap, root, false, templateKeyword);
}

bool cfg::cfgtemp::addTemplates(TemplateMap& templateMap, Value& cfgValue,
		bool removeTemplatesFromCfgValue, const std::string& templateKeyword)
{
	std::string errorMsg;
	return addTemplates(templateMap, cfgValue, removeTemplatesFromCfgValue,
			templateKeyword, errorMsg);
}

bool cfg::cfgtemp::addTemplates(TemplateMap& templateMap, Value& cfgValue,
		bool removeTemplatesFromCfgValue, const std::string& templateKeyword,
		std::string& outErrorMsg)
{
	if (!cfgValue.isObject()) {
		outErrorMsg = "is no object";
		return false;
	}
	std::vector<NameValuePair>& pairs = cfgValue.mObject;
	std::size_t cnt = pairs.size();
	for (std::size_t i = 0; i < cnt; ++i) {
		NameValuePair& nvp = pairs[i];
		if (nvp.mName.mText == templateKeyword) {
			if (!nvp.mValue.isObject()) {
				outErrorMsg = nvp.mValue.getFilenameAndPosition() +
						": is no object";
				return false;
			}
			std::vector<NameValuePair>& pairsFromTmp = nvp.mValue.mObject;
			std::size_t pairTmpCnt = pairsFromTmp.size();
			int namePairIndex = -1;
			for (std::size_t pi = 0; pi < pairTmpCnt; ++pi) {
				if (!pairsFromTmp[pi].isEmptyOrComment()) {
					namePairIndex = static_cast<int>(pi);
					break;
				}
			}
			if (namePairIndex < 0) {
				outErrorMsg = nvp.mValue.getFilenameAndPosition() +
						": two or more pairs necessary";
				return false;
			}
			int paramsPairIndex = -1;
			for (std::size_t pi = namePairIndex + 1; pi < pairTmpCnt; ++pi) {
				if (!pairsFromTmp[pi].isEmptyOrComment()) {
					paramsPairIndex = static_cast<int>(pi);
					break;
				}
			}
			if (paramsPairIndex < 0) {
				outErrorMsg = nvp.mValue.getFilenameAndPosition() +
						": two or more pairs necessary";
				return false;
			}
			NameValuePair& namePair = pairsFromTmp[namePairIndex];
			if (namePair.mName.mText != "name") {
				outErrorMsg = namePair.mName.getFilenameAndPosition() +
						": must be 'name'";
				return false;
			}
			std::string name = namePair.mValue.mText;
			if (name.empty()) {
				outErrorMsg = namePair.mValue.getFilenameAndPosition() +
						": no valid name";
				return false;
			}
			if (templateMap.find(name) != templateMap.end()) {
				outErrorMsg = namePair.mValue.getFilenameAndPosition() +
						": name '" + name + "' is already used";
				return false;
			}
			NameValuePair& paramsPair = pairsFromTmp[paramsPairIndex];
			if (paramsPair.mName.mText != "parameters") {
				outErrorMsg = paramsPair.mName.getFilenameAndPosition() +
						": must be 'parameters'";
				return false;
			}
			std::vector<std::string> parameters;
			if (paramsPair.mValue.isText()) {
				if (paramsPair.mValue.mText != "none") {
					parameters.push_back(paramsPair.mValue.mText);
				}
			}
			else if (paramsPair.mValue.isArray()) {
				std::size_t cnt = paramsPair.mValue.mArray.size();
				parameters.reserve(cnt);
				for (std::size_t i = 0; i < cnt; ++i) {
					if (paramsPair.mValue.mArray[i].mText.empty()) {
						outErrorMsg = paramsPair.mValue.mArray[i].getFilenameAndPosition() +
								": parameter must be a text";
						return false;
					}
					parameters.push_back(paramsPair.mValue.mArray[i].mText);
				}
			}
			else {
				outErrorMsg = paramsPair.mValue.getFilenameAndPosition() +
						": parameter must be a text or an array of texts";
				return false;
			}
#if 0
			// WRONG version
			// This version is problematic.
			// It also includes empty lines and comments to the template pairs
			// which can make problems when the template is used (applied).
			if (!templateMap.insert(TemplateMap::value_type(name, CfgTemplate(
					name, parameters,
					pairsFromTmp.begin() + paramsPairIndex + 1,
					pairsFromTmp.end()))).second) {
				outErrorMsg = namePair.mValue.getFilenameAndPosition() +
						": insert template with name '" + name + "' failed";
				return false;
			}
#else
			// CORRECT version
			// This version makes no problem also if file is loaded with empty lines and comments.
			// The empty lines and comments are checked and not included to the template pairs.
			auto rv = templateMap.insert(
					TemplateMap::value_type(name, CfgTemplate(
					name, parameters)));
			if (!rv.second) {
				outErrorMsg = namePair.mValue.getFilenameAndPosition() +
						": insert template with name '" + name + "' failed";
				return false;
			}
			std::vector<NameValuePair>& tempPairs = rv.first->second.getPairs();
			tempPairs.reserve(pairTmpCnt - (paramsPairIndex + 1));
			for (std::size_t ii = paramsPairIndex + 1; ii < pairTmpCnt; ++ii) {
				if (!pairsFromTmp[ii].isEmptyOrComment()) {
					tempPairs.push_back(pairsFromTmp[ii]);
				}
			}
#endif
			if (removeTemplatesFromCfgValue) {
				pairs.erase(pairs.begin() + i);
				--i;
				--cnt;
			}
		}
	}
	return true;
}

bool cfg::cfgtemp::useTemplates(const TemplateMap& templateMap, Value& cfgValue,
		const std::string& useTemplateKeyword)
{
	std::string errorMsg;
	return useTemplates(templateMap, cfgValue, useTemplateKeyword, false, false, errorMsg);
}

bool cfg::cfgtemp::useTemplates(const TemplateMap& templateMap, Value& cfgValue,
		const std::string& useTemplateKeyword,
		bool checkForInterpreterExpressions,
		bool allowInterpretationWithQuotes,
		std::string& outErrorMsg)
{
	if (!cfgValue.isObject()) {
		return true;
	}
	int pairDiffCount = 0;
	std::vector<std::string> templateNameStack;
	return applyTemplates(templateMap, cfgValue.mObject, -1, -1,
			useTemplateKeyword,
			checkForInterpreterExpressions, allowInterpretationWithQuotes,
			0, templateNameStack,
			pairDiffCount, outErrorMsg);
}

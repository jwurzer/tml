#ifndef TML_CFG_H
#define TML_CFG_H

#include <string>
#include <fstream>
#include <memory>
#include <vector>

/*
 * This file contains the config classes for storing the parsed configuration
 * from a tml file, JSON file, BSON coding, BER/DER coding.
 */
namespace cfg
{
	class NameValuePair;
	class SearchRule;

	class Value
	{
	public:
		enum EValueType
		{
			TYPE_NONE  = 0, // none, nothing, empty (empty line)
			TYPE_NULL,
			TYPE_BOOL,
			TYPE_FLOAT,
			TYPE_INT,
			TYPE_TEXT,
			TYPE_COMMENT, // use mText, e.g. # comment text
			TYPE_ARRAY,
			TYPE_OBJECT,
		};

		// use a shared_ptr instead of direct a std::string to only have one string instance per file
		// independent if std::string implemetation has ref counting or not.
		std::shared_ptr<const std::string> mFilename;
		// -1 for no line number
		int mLineNumber;
		// -1 for no offset. offset is the horizontal offset at the text line from file
		int mOffset;

		/**
		 * if mType is TYPE_FLOAT then the float value is stored at mFloatingPoint
		 * and also stored as rounded integer (up/down rounding at .5) and if
		 * the floating point value is between +-0.5 then mBool is false and
		 * otherwise its true.
		 *
		 * if mType is TYPE_INT then the integer value is stored at mInteger and
		 * also stored as float to mFloatingPoint and if the integer value is
		 * zero then mBool is set to false otherwise its set true.
		 *
		 * if mType is TYPE_BOOL then mBool stores the value. mFloatingPoint
		 * and mInteger would be also 1.0 and 1 for true or 0.0 and 0 for false.
		 *
		 * if mType is TYPE_BOOL, TYPE_FLOAT or TYPE_INT then mText is always
		 * empty (--> "").
		 */
		EValueType mType;

		/**
		 * only for additional informations for TYPE_INT. if mType is
		 * TYPE_INT then mParseBase give infos if it was a dec, octal or
		 * hex string to parse.
		 * if mType is TYPE_FLOAT then mParseBase is always 10.
		 * if mType is TYPE_BOOL then mParseBase is always 1.
		 */
		unsigned int mParseBase;

		bool mBool;
		float mFloatingPoint;
		int mInteger;
		std::string mText;
		std::vector<Value> mArray;
		std::vector<NameValuePair> mObject;

		Value();
		~Value();

		// return line number and offset as :<line-number>:<offset>
		// If only a line number is set then :<line-number> is returned.
		// If only a offset is set then ::<offset> is returned
		// If both line number and offset are -1 then a empty string is returned.
		std::string getFilePosition() const;
		std::string getFilenameAndPosition() const;
		// error happend at
		void printFilePositionAsError() const;
		// clear/reset everything excepted the pointer to mFilename
		void clear();
		void setNull();
		void setBool(bool value);
		void setFloatingPoint(float value);
		void setInteger(int value, unsigned int parseBase = 10);
		void setText(const std::string& text);
		void setComment(const std::string& text);
		void setArray();
		void setObject();
		bool equalText(const std::string& text) const;

		bool isEmpty() const { return mType == TYPE_NONE; }
		bool isComment() const { return mType == TYPE_COMMENT; }
		bool isNull() const { return mType == TYPE_NULL; }
		bool isBool() const { return mType == TYPE_BOOL; }
		bool isFloat() const { return mType == TYPE_FLOAT; }
		bool isInteger() const { return mType == TYPE_INT; }
		bool isNumber() const { return isInteger() || isFloat(); }
		bool isText() const { return mType == TYPE_TEXT; }
		bool isArray() const { return mType == TYPE_ARRAY; }
		bool isNoObject() const { return mType == TYPE_NONE || mType == TYPE_NULL ||
				mType == TYPE_BOOL || mType == TYPE_FLOAT ||
				mType == TYPE_INT || mType == TYPE_TEXT ||
				mType == TYPE_COMMENT || mType == TYPE_ARRAY; }
		bool isObject() const { return mType == TYPE_OBJECT; }

		/**
		 * @param attrName attribute name of a value pair inside a object
		 * @return Return null if none or more than one exist otherwise if
		 *         exactly one attribute with this name exist then this
		 *         value pair is returned.
		 */
		const NameValuePair* objectGetValuePair(const std::string &attrName) const;
		/**
		 * Same as objectGetValuePair() but only return the value of the value pair.
		 */
		const Value* objectGetValue(const std::string &attrName) const;
		bool objectGetText(const std::string &attrName, std::string &attrValue) const;
		// if not exist it return ""
		std::string objectGetText(const std::string &attrName) const;
		bool objectGetInteger(const std::string &attrName, int &attrValue) const;
		// if not exist it return 0
		int objectGetInteger(const std::string &attrName) const;
		bool objectGetBool(const std::string &attrName, bool &attrValue) const;
		bool objectGetBool(const std::string &attrName) const;

		/**
		 * Same as objectGetValuePair() but allows multiple attributes with
		 * the same attribute name.
		 */
		std::vector<const NameValuePair*> objectGetValuePairs(
				const std::string &attrName) const;
		std::vector<const Value*> objectGetValues(const std::string &attrName) const;
		std::vector<const std::string*> objectGetTexts(
				const std::string &attrName) const;
		std::vector<int> objectGetIntegers(const std::string &attrName) const;
		int objectGetAttrIndex(const std::string &attrName) const;
		/**
		 * @param rules The size limit is 64 without the termination rule
		 *              (64 rules + 1 termination rule)
		 * @param allowRandomSequence
		 * @param allowUnusedValuePairs
		 * @param allowEarlyReturn This value is only used wenn allowUnusedValuePairs is false.
		 *        If allowUnusedValuePairs is false and this parameter is true
		 *        then if no rule is found for the current attribute name
		 *        the function can also return successful. It return successful
		 *        if all "must exist" rules already applied and no future
		 *        attribute name has an existing rule at the rule set.
		 * @param allowDuplicatedNames
		 * @param startIndex
		 * @return Return the store count.
		 */
		int objectSearch(const SearchRule *rules, bool allowRandomSequence,
				bool allowUnusedValuePairs, bool allowEarlyReturn,
				bool allowDuplicatedNames,
				bool allowDuplicatedRuleNamesWithDiffTypes,
				std::size_t startIndex,
				std::size_t *outNextIndex) const;
	};

	/**
	 * A name value pair has always two values. If the first value (name) is
	 * a string then the name value pair is compatible with a JSON object which
	 * must have a string and a value.
	 */
	class NameValuePair
	{
	public:
		Value mName;
		Value mValue;

		// -1 for no deep specified
		int mDeep;

		NameValuePair();
		~NameValuePair();

		void clear();
		void setTextBool(const std::string& attrName, bool attrValue);
		void setTextFloat(const std::string& attrName, float attrValue);
		void setTextInt(const std::string& attrName, int attrValue);
		void setTextText(const std::string& attrName, const std::string& attrValue);
		void setObject();
		void setObject(const std::string &objectName);
		bool isEmpty() const { return mName.mType == Value::TYPE_NONE &&
				mValue.mType == Value::TYPE_NONE; }
		bool isComment() const { return mName.mType == Value::TYPE_COMMENT &&
				mValue.mType == Value::TYPE_NONE; }
		bool isEmptyOrComment() const { return
				(mName.mType == Value::TYPE_NONE ||
				mName.mType == Value::TYPE_COMMENT) &&
				mValue.mType == Value::TYPE_NONE; }
		bool isObject() const { return mName.isNoObject() &&
					mValue.isObject(); }
		bool isEmptyObject() const { return mName.isEmpty() &&
					mValue.isObject() && mValue.mObject.empty(); }
	};

	class SearchRule
	{
	public:
		const std::string mName;

		enum EStoreType
		{
			TYPE_UNKNOWN = 0,

			TYPE_NULL,
			TYPE_BOOL,
			TYPE_FLOAT,
			TYPE_DOUBLE,
			TYPE_INT,
			TYPE_UINT,
			TYPE_STRING,
			TYPE_OBJECT,
			TYPE_VALUE,
			TYPE_VALUE_PAIR,
		} mType;

		union {
			void* mPtr; // used to reset pointer to null
			bool* mNull;
			bool* mBool;
			float* mFloat;
			double* mDouble;
			int* mInt;
			unsigned int* mUInt;
			std::string* mStr;
			const std::vector<NameValuePair>** mObject;
			const Value** mValue;
			const NameValuePair** mValuePair;
		} mStorePtr;

		enum ERule
		{
			RULE_OPTIONAL = 0,
			RULE_MUST_EXIST,
		} mRule;

		enum EAllowedTypes
		{
			ALLOW_NONE   =  1,
			ALLOW_NULL   =  2,
			ALLOW_BOOL   =  4,
			ALLOW_FLOAT  =  8,
			ALLOW_INT    = 16,
			ALLOW_TEXT   = 32,
			ALLOW_OBJECT = 64,

			ALLOW_NUMBER = (ALLOW_FLOAT | ALLOW_INT),
			ALLOW_ALL    = (ALLOW_NONE | ALLOW_NULL | ALLOW_BOOL | ALLOW_FLOAT | ALLOW_INT | ALLOW_TEXT | ALLOW_OBJECT),
		};
		unsigned int mAllowedTypes; // as uint instead of AllowedTypes because its used as flags

		// stores the count how often the rule was used
		unsigned int* mUsedCount;

		SearchRule(const char* name) // should be always "" for EOF of rule array
				:mName(name),
				mType(TYPE_UNKNOWN), mRule(RULE_OPTIONAL),
				mAllowedTypes(0),
				mUsedCount(nullptr) {}

		SearchRule(const char* name, bool* boolPtr, ERule rule,
				unsigned int typeFlags = ALLOW_BOOL, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_BOOL), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mBool = boolPtr; }
		SearchRule(const std::string& name, bool* boolPtr, ERule rule,
				unsigned int typeFlags = ALLOW_BOOL, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_BOOL), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mBool = boolPtr; }

		SearchRule(const char* name, float* floatPtr, ERule rule,
				unsigned int typeFlags = ALLOW_NUMBER, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_FLOAT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mFloat = floatPtr; }
		SearchRule(const std::string& name, float* floatPtr, ERule rule,
				unsigned int typeFlags = ALLOW_NUMBER, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_FLOAT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mFloat = floatPtr; }

		SearchRule(const char* name, double* doublePtr, ERule rule,
				unsigned int typeFlags = ALLOW_NUMBER, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_DOUBLE), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mDouble = doublePtr; }
		SearchRule(const std::string& name, double* doublePtr, ERule rule,
				unsigned int typeFlags = ALLOW_NUMBER, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_DOUBLE), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mDouble = doublePtr; }

		SearchRule(const char* name, int* intPtr, ERule rule,
				unsigned int typeFlags = ALLOW_INT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_INT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mInt = intPtr; }
		SearchRule(const std::string& name, int* intPtr, ERule rule,
				unsigned int typeFlags = ALLOW_INT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_INT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mInt = intPtr; }

		SearchRule(const char* name, unsigned int* uintPtr, ERule rule,
				unsigned int typeFlags = ALLOW_INT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_UINT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mUInt = uintPtr; }
		SearchRule(const std::string& name, unsigned int* uintPtr, ERule rule,
				unsigned int typeFlags = ALLOW_INT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_UINT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mUInt = uintPtr; }

		SearchRule(const char* name, std::string* strPtr, ERule rule,
				unsigned int typeFlags = ALLOW_TEXT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_STRING), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mStr = strPtr; }
		SearchRule(const std::string& name, std::string* strPtr, ERule rule,
				unsigned int typeFlags = ALLOW_TEXT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_STRING), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mStr = strPtr; }

		SearchRule(const char* name, const std::vector<NameValuePair>** arrayPtr, ERule rule,
				unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_OBJECT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mObject = arrayPtr; }
		SearchRule(const std::string& name, const std::vector<NameValuePair>** arrayPtr, ERule rule,
				unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_OBJECT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mObject = arrayPtr; }

		SearchRule(const char* name, const Value** arrayPtr, ERule rule,
				unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_VALUE), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
		{ mStorePtr.mValue = arrayPtr; }
		SearchRule(const std::string& name, const Value** arrayPtr, ERule rule,
				unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_VALUE), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
		{ mStorePtr.mValue = arrayPtr; }

		SearchRule(const char* name, const NameValuePair** valuePairPtr, ERule rule,
					unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				 mType(TYPE_VALUE_PAIR), mRule(rule),
				 mAllowedTypes(typeFlags),
				 mUsedCount(usedCount)
		{ mStorePtr.mValuePair = valuePairPtr; }
		SearchRule(const std::string& name, const NameValuePair** valuePairPtr, ERule rule,
					unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				 mType(TYPE_VALUE_PAIR), mRule(rule),
				 mAllowedTypes(typeFlags),
				 mUsedCount(usedCount)
		{ mStorePtr.mValuePair = valuePairPtr; }
	};
}
#endif

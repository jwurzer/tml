#ifndef CFG_CFG_H
#define CFG_CFG_H

#include <cfg/export.h>

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
	class SelectRule;

	enum class EReset
	{
		RESET_NOTHING = 0,
		RESET_POINTERS_TO_NULL,
		/* reset pointers to null and all other values to zero or false. */
		RESET_EVERYTHING_TO_DEFAULTS,
	};

	class CFG_API Value
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
		// -1 for no deep. deep (indent count) of the line.
		// e.g. '[tab][tab]abc = xyz' has a deep of 2 for both the name abc and the value xyz.
		// Nvp means name-value-paar and should be a hint that's the deep of the line
		// and not the deep of the beginning from the real value (which could be the same but must not).
		int mNvpDeep;

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
		explicit Value(bool boolValue,
				int lineNumber = -1, int offset = -1, int nvpDeep = -1,
				const std::shared_ptr<const std::string>& filename = nullptr);
		explicit Value(float floatingPointValue,
				int lineNumber = -1, int offset = -1, int nvpDeep = -1,
				const std::shared_ptr<const std::string>& filename = nullptr);
		explicit Value(int integerValue, unsigned int parseBase,
				int lineNumber = -1, int offset = -1, int nvpDeep = -1,
				const std::shared_ptr<const std::string>& filename = nullptr);
		explicit Value(const std::string& text,
				int lineNumber = -1, int offset = -1, int nvpDeep = -1,
				const std::shared_ptr<const std::string>& filename = nullptr);
		explicit Value(const std::vector<Value>& array,
				int lineNumber = -1, int offset = -1, int nvpDeep = -1,
				const std::shared_ptr<const std::string>& filename = nullptr);
		explicit Value(const std::vector<NameValuePair>& object,
				int lineNumber = -1, int offset = -1, int nvpDeep = -1,
				const std::shared_ptr<const std::string>& filename = nullptr);
		Value(Value&& other);
		Value& operator=(Value&& other);
		Value(const Value& other) = default;
		Value& operator=(const Value& other) = default;
		//~Value(); // default destructor is enougth

		// return line number and offset as :<line-number>:<offset>
		// If only a line number is set then :<line-number> is returned.
		// If only a offset is set then ::<offset> is returned
		// If both line number and offset are -1 then a empty string is returned.
		std::string getFilePosition() const;
		std::string getFilenameAndPosition() const;
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
		/**
		 * Check if it is a "simple" value.
		 * A simple value is != a primitive value.
		 * A value which can be represented by a single value (no object, no array, no none).
		 * String is a simple value.
		 * TYPE_NONE is NOT a simple value!
		 * TYPE_COMMENT is NOT a simple value!
		 * @return Return true if it is simple.
		 */
		bool isSimple() const { return mType == TYPE_NULL ||
					mType == TYPE_BOOL || mType == TYPE_FLOAT ||
					mType == TYPE_INT || mType == TYPE_TEXT; }
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
		 * objectGet() is to check and get various name value pairs of the object
		 * by a rule-based schema.
		 *
		 * @param rules The size limit is 64 without the termination rule
		 *              (64 rules + 1 termination rule)
		 * @param allowRandomSequence If true then the name-value pairs don't
		 *        need the same order as the select rules. If false then
		 *        the name-value pair must have the same order as the select rules.
		 * @param allowUnusedValuePairs If true then name-value pairs can be
		 *        skipped if no matching select-rules are found.
		 *        If false and a name-value pair has no select-rule then
		 *        this function will fail.
		 * @param allowEarlyReturn This value is only used when allowUnusedValuePairs is false.
		 *        If allowUnusedValuePairs is false and this parameter is true
		 *        then if no rule is found for the current attribute name
		 *        the function can also return successful. It return successful
		 *        if all "must exist" rules already applied and no future
		 *        attribute name has an existing rule at the rule set.
		 * @param allowDuplicatedNames True means that a select-rule from the
		 *        rules array is allowed to be used multiple times.
		 *        This means that more than one name-value pair with the
		 *        same name and the same value type is allowed.
		 *        This doesn't mean that the rules-array can have multiple
		 *        rules with the same name. This only means that a rule can
		 *        be used multiple times by parsing the config object.
		 *        If the rule is applied multiple times then the value of
		 *        the first name-value pair is stored. The other values are
		 *        discarded/dropped.
		 * @param allowDuplicatedRuleNamesWithDiffTypes Means that the
		 *        rules-array can have multiple rules with the same name.
		 *        The type should be different because otherwise it make no
		 *        sense.
		 *        This doesn't mean that a rule with a specific type can
		 *        be used multiple times by parsing the config object. For
		 *        this case allowDuplicatesNames exist.
		 *        The parameters allowDuplicatedRuleNamesWithDiffTypes and
		 *        allowDuplicatesNames can be combined but does not have to
		 *        be combined.
		 * @param startIndex Start index of the first name-value pair.
		 *        The name-value pairs before the start index are skipped
		 *        for the select rules. Skipping select rules with start
		 *        index is NOT possible. The start index refers to the
		 *        start position of the name-value pairs and not to the
		 *        select rules.
		 * @param outNextIndex Can be null. If not null then the next unused
		 *        index of name value pairs is stored. This is the index +1
		 *        after the last used name-value pair (highest used index).
		 * @param reset Define if the values for storing should be reset to
		 *        defaults or only pointers should be set to null or nothing
		 *        should happend before parsing.
		 * @param errMsg Can be null. If not null then the error message is
		 *        stored (if an error happend).
		 * @return Return the store count.
		 */
		int objectGet(const SelectRule *rules, bool allowRandomSequence,
				bool allowUnusedValuePairs, bool allowEarlyReturn,
				bool allowDuplicatedNames,
				bool allowDuplicatedRuleNamesWithDiffTypes,
				std::size_t startIndex,
				std::size_t *outNextIndex,
				EReset reset = EReset::RESET_POINTERS_TO_NULL,
				std::string* errMsg = nullptr,
				std::string* warnings = nullptr) const;
	};

	Value none(int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value nullValue(int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value boolValue(bool boolValue,
			int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value floatValue(float floatingPointValue,
			int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value intValue(int integerValue, unsigned int parseBase = 10,
			int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value text(const std::string& text,
			int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value commentValue(const std::string& comment,
			int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value array(int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value array(const std::vector<Value>& array,
			int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value object(int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);
	Value object(const std::vector<NameValuePair>& object,
			int lineNumber = -1, int offset = -1, int nvpDeep = -1,
			const std::shared_ptr<const std::string>& filename = nullptr);

	/**
	 * free function version of Value::objectGet. This version is
	 * useful if you only the access to the array Value::mObject of the value
	 * which stores the object but not access to the value itself.
	 */
	int objectGet(const std::vector<NameValuePair>& nvpairsOfObject,
			const SelectRule *rules, bool allowRandomSequence,
			bool allowUnusedValuePairs, bool allowEarlyReturn,
			bool allowDuplicatedNames,
			bool allowDuplicatedRuleNamesWithDiffTypes,
			std::size_t startIndex,
			std::size_t *outNextIndex,
			EReset reset = EReset::RESET_POINTERS_TO_NULL,
			std::string* errMsg = nullptr,
			std::string* warnings = nullptr);

	/**
	 * A name value pair has a name and a value. But this name value pair type
	 * uses for both, for the name and the value, the same type. The type Value
	 * is used for the name and the value which means that the name does not
	 * have to be a string. The name can also be a integer, boolean etc.
	 * If the name is a string then the name value pair is compatible
	 * with the entries of a JSON object which must have strings as names.
	 */
	class CFG_API NameValuePair
	{
	public:
		Value mName;
		Value mValue;

		// -1 for no deep specified
		int mDeep;

		NameValuePair();
		NameValuePair(const Value& name, const Value& value, int deep = -1);
		NameValuePair(NameValuePair&& other);
		NameValuePair& operator=(NameValuePair&& other);
		NameValuePair(const NameValuePair& other) = default;
		NameValuePair& operator=(const NameValuePair& other) = default;
		//~NameValuePair(); // default destructor is enougth

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

	NameValuePair empty(int deep = -1);
	NameValuePair comment(const std::string& comment, int deep = -1);
	NameValuePair single(const Value& name, int deep = -1);
	NameValuePair pair(const Value& name, const Value& value, int deep = -1);

	class CFG_API SelectRule
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
			TYPE_ARRAY,
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
			const std::vector<Value>** mArray;
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
			ALLOW_NONE   =   1,
			ALLOW_NULL   =   2,
			ALLOW_BOOL   =   4,
			ALLOW_FLOAT  =   8,
			ALLOW_INT    =  16,
			ALLOW_TEXT   =  32,
			ALLOW_ARRAY  =  64,
			ALLOW_OBJECT = 128,

			ALLOW_NUMBER = (ALLOW_FLOAT | ALLOW_INT),
			ALLOW_ALL    = (ALLOW_NONE | ALLOW_NULL | ALLOW_BOOL | ALLOW_FLOAT |
					ALLOW_INT | ALLOW_TEXT | ALLOW_ARRAY | ALLOW_OBJECT),
		};
		unsigned int mAllowedTypes; // as uint instead of AllowedTypes because its used as flags

		// stores the count how often the rule was used
		unsigned int* mUsedCount;

		SelectRule(const char* name) // should be always "" for EOF of rule array
				:mName(name),
				mType(TYPE_UNKNOWN), mRule(RULE_OPTIONAL),
				mAllowedTypes(0),
				mUsedCount(nullptr) {}

		SelectRule(const char* name, bool* boolPtr, ERule rule,
				unsigned int typeFlags = ALLOW_BOOL, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_BOOL), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mBool = boolPtr; }
		SelectRule(const std::string& name, bool* boolPtr, ERule rule,
				unsigned int typeFlags = ALLOW_BOOL, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_BOOL), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mBool = boolPtr; }

		static SelectRule nullRule(const char* name, bool* boolPtr, ERule rule,
				unsigned int typeFlags = ALLOW_NULL, unsigned int* usedCount = nullptr)
		{
			SelectRule sr(name, boolPtr, rule, typeFlags, usedCount);
			sr.mType = TYPE_NULL;
			sr.mStorePtr.mBool = nullptr; // makes not really a sense because its an union. mBool and mNull have the same memory space.
			sr.mStorePtr.mNull = boolPtr; // important that mNull is set after mBool is set... because of union :-P (or simple remove the line above)
			return sr;
		}
		static SelectRule nullRule(const std::string& name, bool* boolPtr, ERule rule,
				unsigned int typeFlags = ALLOW_NULL, unsigned int* usedCount = nullptr)
		{
			SelectRule sr(name, boolPtr, rule, typeFlags, usedCount);
			sr.mType = TYPE_NULL;
			sr.mStorePtr.mBool = nullptr; // makes not really a sense because its an union. mBool and mNull have the same memory space.
			sr.mStorePtr.mNull = boolPtr; // important that mNull is set after mBool is set... because of union :-P (or simple remove the line above)
			return sr;
		}

		SelectRule(const char* name, float* floatPtr, ERule rule,
				unsigned int typeFlags = ALLOW_NUMBER, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_FLOAT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mFloat = floatPtr; }
		SelectRule(const std::string& name, float* floatPtr, ERule rule,
				unsigned int typeFlags = ALLOW_NUMBER, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_FLOAT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mFloat = floatPtr; }

		SelectRule(const char* name, double* doublePtr, ERule rule,
				unsigned int typeFlags = ALLOW_NUMBER, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_DOUBLE), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mDouble = doublePtr; }
		SelectRule(const std::string& name, double* doublePtr, ERule rule,
				unsigned int typeFlags = ALLOW_NUMBER, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_DOUBLE), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mDouble = doublePtr; }

		SelectRule(const char* name, int* intPtr, ERule rule,
				unsigned int typeFlags = ALLOW_INT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_INT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mInt = intPtr; }
		SelectRule(const std::string& name, int* intPtr, ERule rule,
				unsigned int typeFlags = ALLOW_INT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_INT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mInt = intPtr; }

		SelectRule(const char* name, unsigned int* uintPtr, ERule rule,
				unsigned int typeFlags = ALLOW_INT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_UINT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mUInt = uintPtr; }
		SelectRule(const std::string& name, unsigned int* uintPtr, ERule rule,
				unsigned int typeFlags = ALLOW_INT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_UINT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mUInt = uintPtr; }

		SelectRule(const char* name, std::string* strPtr, ERule rule,
				unsigned int typeFlags = ALLOW_TEXT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_STRING), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mStr = strPtr; }
		SelectRule(const std::string& name, std::string* strPtr, ERule rule,
				unsigned int typeFlags = ALLOW_TEXT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_STRING), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mStr = strPtr; }

		/**
		 * Select rule for a value which holds an array. The array of the value
		 * Value::mArray can be referenced by arrayPtr (after a successful
		 * objectGet() call).
		 */
		SelectRule(const char* name, const std::vector<Value>** arrayPtr, ERule rule,
				unsigned int typeFlags = ALLOW_ARRAY, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_ARRAY), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
		{ mStorePtr.mArray = arrayPtr; }
		SelectRule(const std::string& name, const std::vector<Value>** arrayPtr, ERule rule,
				unsigned int typeFlags = ALLOW_ARRAY, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_ARRAY), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
		{ mStorePtr.mArray = arrayPtr; }

		/**
		 * Select a value (which is an object) of a name-value-pair and
		 * references to its name-value-pair array Value::mObject.
		 * If you want to use objectGet() for the received object in objPtr
		 * then you have to use the free function version cfg::objectGet().
		 * The member function cfg::Value::objectGet() can't be used because
		 * not the object value itself is referenced in objPtr. Only the
		 * array to the name-value-pairs of the object.
		 */
		SelectRule(const char* name, const std::vector<NameValuePair>** objPtr, ERule rule,
				unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_OBJECT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mObject = objPtr; }
		SelectRule(const std::string& name, const std::vector<NameValuePair>** objPtr, ERule rule,
				unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_OBJECT), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
				{ mStorePtr.mObject = objPtr; }

		/**
		 * Select a value of a name-value-pair. Per default the
		 * value of the name-value-pair must be an object. But this rule
		 * can be also used to search for a value of any other type (if
		 * typeFlags is set correctly). e.g. Can also be used to search for
		 * a value which itself stores an array.
		 * @note: For objects and arrays there is also another rule (Constructor)
		 * available.
		 */
		SelectRule(const char* name, const Value** valuePtr, ERule rule,
				unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_VALUE), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
		{ mStorePtr.mValue = valuePtr; }
		SelectRule(const std::string& name, const Value** valuePtr, ERule rule,
				unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				mType(TYPE_VALUE), mRule(rule),
				mAllowedTypes(typeFlags),
				mUsedCount(usedCount)
		{ mStorePtr.mValue = valuePtr; }

		/**
		 * Select a value of a name-value-pair but not only references to
		 * the value. Instead its references to the whole name-value-pair.
		 * Per default this rule allows only an object. But it can be used
		 * for any other type.
		 */
		SelectRule(const char* name, const NameValuePair** valuePairPtr, ERule rule,
					unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				 mType(TYPE_VALUE_PAIR), mRule(rule),
				 mAllowedTypes(typeFlags),
				 mUsedCount(usedCount)
		{ mStorePtr.mValuePair = valuePairPtr; }
		SelectRule(const std::string& name, const NameValuePair** valuePairPtr, ERule rule,
					unsigned int typeFlags = ALLOW_OBJECT, unsigned int* usedCount = nullptr)
				:mName(name),
				 mType(TYPE_VALUE_PAIR), mRule(rule),
				 mAllowedTypes(typeFlags),
				 mUsedCount(usedCount)
		{ mStorePtr.mValuePair = valuePairPtr; }
	};
}
#endif

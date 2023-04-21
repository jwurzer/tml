#ifndef CFG_CFG_SCHEMA_H
#define CFG_CFG_SCHEMA_H

#include <cfg/export.h>
#include <cfg/cfg.h>
#include <string>

namespace cfg
{
	class NVPSchema;

	/**
	 * nv-fragment is a name or value of a name-value pair.
	 * A nv-fragment schema is a validation rule for a nv-fragment.
	 */
	class CFG_API NVFragmentSchema
	{
	public:
		unsigned int mAllowedTypes = 0; // as uint instead of AllowedTypes because its used as flags

		bool mValueIsUsed = false;
		Value mValue;
		bool mDefaultIsUsed = false;
		Value mDefault;

		bool mFloatMinUsed = false;
		float mFloatMin = 0;
		bool mFloatMaxUsed = false;
		float mFloatMax = 0;

		bool mIntMinUsed = false;
		int mIntMin = 0;
		bool mIntMaxUsed = false;
		int mIntMax = 0;

		std::vector<NVFragmentSchema> mArray;
		std::vector<NVPSchema> mObject;

		void clear();
	};

	class CFG_API NVPSchema
	{
	public:
		NVFragmentSchema mName;
		bool mValueIsUsed = false;
		NVFragmentSchema mValue;

		void clear()
		{
			mName.clear();
			mValueIsUsed = false;
			mValue.clear();
		}
	};

	namespace schema
	{
		CFG_API
		bool getSchemaFromCfgValue(NVFragmentSchema& outNvfs, const Value& cfgValue,
				std::string& outErrorMsg);

		CFG_API
		bool getSchemaAsCfgValue(Value& outCfgValue,
				const NVFragmentSchema& nvfs, std::string& outErrorMsg);
	}
}

#endif

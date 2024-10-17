#include "IConvert.h"

class CPUConvert : public IConvert
{
public:
	void PreflightData(uint8_t* bytes, IConvertData** data, int32_t arrayLength)
	{
		auto localData = new CPUIConvertData(arrayLength);

		for (auto i = 0; i < arrayLength; i++)
			localData->Shorts[i] = bytes[i];

		*data = localData;
	}

	float MeanSquaredError(IConvertData* lData, IConvertData* rData, int32_t arrayLength)
	{
		float sum = 0.0;
		auto lCpuData = static_cast<CPUIConvertData*>(lData);
		auto rCpuData = static_cast<CPUIConvertData*>(rData);

		for (auto i = 0; i < arrayLength; i++) {
			auto difference = (lCpuData->Shorts[i] - rCpuData->Shorts[i]);
			sum = sum + difference * difference;
		}

		return sum / arrayLength;
	}
};

IConvert* cpuConvert = new CPUConvert();
IConvert* CPUBasedIConvert()
{
	return cpuConvert;
}
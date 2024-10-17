#include "IConvert.h"
#include <string.h>
#include <exception>

class CPUConvert : public IConvert
{
public:
	IConvertData* PreflightData(uint8_t* bytes, int32_t width, int32_t height, int32_t bytesPerChannel)
	{
		auto localData = new CPUConvertData<uint8_t>(width * height * bytesPerChannel, width, height, bytesPerChannel);
		memcpy(localData->Values, bytes, localData->ValuesLength);
		return localData;
	}

	double MeanSquaredError(IConvertData* lData, IConvertData* rData)
	{
		VALIDATE_AND_EXTRACT(CPUConvertData<uint8_t>, lCpuData, rCpuData, arrayLength, bytesPerChannel, lData, rData);
		
		int64_t result = 0;
		for (auto i = 0; i < arrayLength; i++)
		{
			auto difference = static_cast<int64_t>((static_cast<int32_t>(lCpuData->Values[i]) - static_cast<int32_t>(rCpuData->Values[i])));
			result += difference * difference;
		}

		return result / (double)arrayLength;
	}
};

IConvert* cpuConvert = new CPUConvert();
IConvert* CPUBasedIConvert()
{
	return cpuConvert;
}
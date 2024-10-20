#include <smmintrin.h>
#include "IConvert.h"
#include <omp.h>

const int registerWidthInBytes = 16;
const int halfRegisterWidth = registerWidthInBytes / 2;

class SSEConvert : public IConvert
{
private:
	inline int HorizontalSum(__m128i vec)
	{
		auto hadd = _mm_hadd_epi32(vec, vec);
		hadd = _mm_hadd_epi32(hadd, hadd);
		return _mm_extract_epi32(hadd, 0);
	}

public:
	IConvertData* PreflightData(uint8_t* bytes, int32_t width, int32_t height, int32_t bytesPerChannel)
	{
		auto localData = new CPUConvertData<__m128i>(16, width * height, width, height, bytesPerChannel);

		#pragma omp parallel for
		for (auto i = 0; i < localData->ValuesLength; i++)
		{
			auto offset = i * 4;
			localData->Values[i] = _mm_set_epi32(bytes[offset], bytes[offset + 1], bytes[offset + 2], bytes[offset + 3]); //Currently only bytesPerChannel = 4 supported.
		}

		return localData;
	}


	double MeanSquaredError(IConvertData* lData, IConvertData* rData)
	{
		VALIDATE_AND_EXTRACT(CPUConvertData<__m128i>, lCpuData, rCpuData, arrayLength, bytesPerChannel, lData, rData);

		int64_t result = 0;
		for (auto i = 0; i < arrayLength; i++)
		{
			auto diff = _mm_sub_epi32(lCpuData->Values[i], rCpuData->Values[i]);
			auto pow = _mm_mullo_epi32(diff, diff);

			result += static_cast<int64_t>(HorizontalSum(pow));
		}

		return result / (double)(arrayLength * bytesPerChannel);
	}
};

IConvert* sseConvert = new SSEConvert();
IConvert* SSEBasedIConvert()
{
	return sseConvert;
}
#include <immintrin.h>
#include "IConvert.h"
#include <omp.h>

const int registerWidthInBytes = 32;
const int halfRegisterWidth = registerWidthInBytes / 2;

static const auto shiftMask = _mm256_setr_epi32(1, 0, 3, 2, 5, 4, 7, 6);

class AVXConvert : public IConvert
{
public:
	IConvertData* PreflightData(uint8_t* bytes, int32_t width, int32_t height, int32_t bytesPerChannel)
	{
		auto localData = new CPUConvertData<__m256i>(32, (width * height) / 2, width, height, bytesPerChannel);
		
		#pragma omp parallel for
		for (auto i = 0; i < localData->ValuesLength; i++)
		{
			auto offset = i * 8;
			localData->Values[i] = _mm256_set_epi32(bytes[offset], bytes[offset + 1], bytes[offset + 2], bytes[offset + 3], bytes[offset + 4], bytes[offset + 5], bytes[offset + 6], bytes[offset + 7]); //Currently only bytesPerChannel = 4 supported.
		}

		return localData;
	}

	double MeanSquaredError(IConvertData* lData, IConvertData* rData)
	{
		__declspec(align(registerWidthInBytes)) int64_t resultBuffer[4] = { 0 };

		VALIDATE_AND_EXTRACT(CPUConvertData<__m256i>, lCpuData, rCpuData, arrayLength, bytesPerChannel, lData, rData);

		int64_t result = 0;
		for (auto i = 0; i < arrayLength; i++)
		{
			auto diff = _mm256_sub_epi32(lCpuData->Values[i], rCpuData->Values[i]);
			auto powlo = _mm256_mul_epi32(diff, diff);
			auto diffhi = _mm256_permutevar8x32_epi32(diff, shiftMask);
			auto powhi = _mm256_mul_epi32(diffhi, diffhi);

			_mm256_store_si256((__m256i*)resultBuffer, powlo);

			for (auto b = 0; b < bytesPerChannel; b++)
				result += resultBuffer[b];

			_mm256_store_si256((__m256i*)resultBuffer, powhi);

			for (auto b = 0; b < bytesPerChannel; b++)
				result += resultBuffer[b];
		}

		return result / (double)(arrayLength * 2 * bytesPerChannel);
	}
};

IConvert* avxConvert = new AVXConvert();
IConvert* AVXBasedIConvert()
{
	return avxConvert;
}
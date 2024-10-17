#include <immintrin.h>
#include "IConvert.h"

const int registerWidthInBytes = 32;
const int halfRegisterWidth = registerWidthInBytes / 2;

static const auto zero = _mm256_setzero_si256();
static const auto shiftMask = _mm256_setr_epi32(1, 0, 3, 2, 5, 4, 7, 6);

class AVXConvert : public IConvert
{
public:
	IConvertData* PreflightData(uint8_t* bytes, int32_t width, int32_t height, int32_t bytesPerChannel)
	{
		auto byteLength = width * height * bytesPerChannel;
		auto localData = new CPUConvertData<__m256i>((width * height) / 2, width, height, bytesPerChannel);
		auto bytesPerChannel2x = bytesPerChannel * 2;
		for (auto i = 0, v = 0; i < byteLength; i += bytesPerChannel2x, v++)
		{
			__declspec(align(registerWidthInBytes)) int32_t ints[8] = { 0 };
			for (auto b = 0; b < bytesPerChannel2x; b++)
				ints[b] = bytes[i + b];

			localData->Values[v] = _mm256_load_si256((__m256i*)ints);
		}

		return localData;
	}

	double MeanSquaredError(IConvertData* lData, IConvertData* rData)
	{
		VALIDATE_AND_EXTRACT(CPUConvertData<__m256i>, lCpuData, rCpuData, arrayLength, bytesPerChannel, lData, rData);

		int64_t result = 0;
		for (auto i = 0; i < arrayLength; i++)
		{
			__declspec(align(registerWidthInBytes)) int64_t resultBuffer[4] = { 0 };
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
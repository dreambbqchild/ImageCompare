#include <immintrin.h>
#include "IConvert.h"

const int registerWidthInBytes = 32;
const int halfRegisterWidth = registerWidthInBytes / 2;

static const auto zero = _mm256_setzero_si256();

class AVXConvert : public IConvert
{
public:
	void PreflightData(uint8_t* bytes, IConvertData** data, int32_t arrayLength)
	{
		auto localData = new CPUIConvertData(arrayLength);
		auto avxLength = arrayLength - (arrayLength % registerWidthInBytes);
		for (auto i = 0; i < avxLength; i += registerWidthInBytes)
		{
			auto ptr = &bytes[i];
			auto shortLo = &localData->Shorts[i];
			auto shortHi = &localData->Shorts[i + halfRegisterWidth];
			auto chunk = _mm256_loadu_si256((__m256i*)ptr);
			auto lo = _mm256_unpacklo_epi8(chunk, zero);
			auto hi = _mm256_unpackhi_epi8(chunk, zero);

			_mm256_storeu_si256((__m256i*) shortLo, lo);
			_mm256_storeu_si256((__m256i*) shortHi, hi);
		}

		for (auto i = avxLength; i < arrayLength; i++)
			localData->Shorts[i] = bytes[i];

		*data = localData;
	}

	float MeanSquaredError(IConvertData* lData, IConvertData* rData, int32_t arrayLength)
	{
		__declspec(align(registerWidthInBytes)) uint32_t resultBuffer[8] = { 0 };
		auto lCpuData = static_cast<CPUIConvertData*>(lData);
		auto rCpuData = static_cast<CPUIConvertData*>(rData);

		auto result = _mm256_setzero_si256();
		auto avxLength = arrayLength - (arrayLength % halfRegisterWidth);
		auto lPtr = lCpuData->Shorts;
		auto rPtr = rCpuData->Shorts;
		int64_t sum = 0;

		for (auto i = 0; i < avxLength; i += halfRegisterWidth, lPtr += halfRegisterWidth, rPtr += halfRegisterWidth)
		{
			auto lChunk = _mm256_loadu_si256((__m256i*)lPtr);
			auto rChunk = _mm256_loadu_si256((__m256i*)rPtr);

			auto value = _mm256_sub_epi16(lChunk, rChunk);
			value = _mm256_mullo_epi16(value, value);

			auto lo = _mm256_unpacklo_epi16(value, zero);
			auto hi = _mm256_unpackhi_epi16(value, zero);

			result = _mm256_add_epi32(result, lo);
			result = _mm256_add_epi32(result, hi);
		}

		_mm256_store_si256((__m256i*)resultBuffer, result);

		for (auto i = 0; i < 8; i++)
			sum += resultBuffer[i];

		for (auto i = avxLength; i < arrayLength; i++)
		{
			auto value = lCpuData->Shorts[i] - rCpuData->Shorts[i];
			sum += value * value;
		}

		return sum / (float)arrayLength;
	}
};

IConvert* avxConvert = new AVXConvert();
IConvert* AVXBasedIConvert()
{
	return avxConvert;
}
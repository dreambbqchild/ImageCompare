#include <emmintrin.h>
#include "IConvert.h"

const int registerWidthInBytes = 16;
const int halfRegisterWidth = registerWidthInBytes / 2;

static const auto zero = _mm_setzero_si128();

class SSEConvert : public IConvert
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
			auto chunk = _mm_loadu_si128((__m128i*)ptr);
			auto lo = _mm_unpacklo_epi8(chunk, zero);
			auto hi = _mm_unpackhi_epi8(chunk, zero);			

			_mm_storeu_si128((__m128i*) shortLo, lo);
			_mm_storeu_si128((__m128i*) shortHi, hi);
		}

		for (auto i = avxLength; i < arrayLength; i++)
			localData->Shorts[i] = bytes[i];

		*data = localData;
	}

	float MeanSquaredError(IConvertData* lData, IConvertData* rData, int32_t arrayLength)
	{
		__declspec(align(registerWidthInBytes)) uint32_t resultBuffer[4] = { 0 };
		auto lCpuData = static_cast<CPUIConvertData*>(lData);
		auto rCpuData = static_cast<CPUIConvertData*>(rData);

		auto result = _mm_setzero_si128();
		auto sseLength = arrayLength - (arrayLength % halfRegisterWidth);
		auto lPtr = lCpuData->Shorts;
		auto rPtr = rCpuData->Shorts;
		int64_t sum = 0;
		
		for (auto i = 0; i < sseLength; i += halfRegisterWidth, lPtr += halfRegisterWidth, rPtr += halfRegisterWidth)
		{
			auto lChunk = _mm_loadu_si128((__m128i*)lPtr);
			auto rChunk = _mm_loadu_si128((__m128i*)rPtr);		

			auto value = _mm_sub_epi16(lChunk, rChunk);
			value = _mm_mullo_epi16(value, value);

			auto lo = _mm_unpacklo_epi16(value, zero);
			auto hi = _mm_unpackhi_epi16(value, zero);

			result = _mm_add_epi32(result, lo);
			result = _mm_add_epi32(result, hi);
		}

		_mm_store_si128((__m128i*)resultBuffer, result);

		for (auto i = 0; i < 4; i++)
			sum += resultBuffer[i];

		for (auto i = sseLength; i < arrayLength; i++)
		{
			auto value = lCpuData->Shorts[i] - rCpuData->Shorts[i];
			sum += value * value;
		}

		return sum / (float)arrayLength;
	}
};

IConvert* sseConvert = new SSEConvert();
IConvert* SSEBasedIConvert()
{
	return sseConvert;
}
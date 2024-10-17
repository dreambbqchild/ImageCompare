#include <smmintrin.h>
#include "IConvert.h"

const int registerWidthInBytes = 16;
const int halfRegisterWidth = registerWidthInBytes / 2;

class SSEConvert : public IConvert
{
public:
	IConvertData* PreflightData(uint8_t* bytes, int32_t width, int32_t height, int32_t bytesPerChannel)
	{
		auto byteLength = width * height * bytesPerChannel;
		auto localData = new CPUConvertData<__m128i>(width * height, width, height, bytesPerChannel);
		for (auto i = 0, v = 0; i < byteLength; i += bytesPerChannel, v++)
		{
			__declspec(align(registerWidthInBytes)) int32_t ints[4] = { 0 };
			for (auto b = 0; b < bytesPerChannel; b++)
				ints[b] = bytes[i + b];

			localData->Values[v] = _mm_loadu_si128((__m128i*)ints);
		}

		return localData;
	}

	double MeanSquaredError(IConvertData* lData, IConvertData* rData)
	{
		VALIDATE_AND_EXTRACT(CPUConvertData<__m128i>, lCpuData, rCpuData, arrayLength, bytesPerChannel, lData, rData);

		int64_t result = 0;
		for (auto i = 0; i < arrayLength; i++)
		{
			__declspec(align(registerWidthInBytes)) int32_t resultBuffer[4] = { 0 };
			auto diff = _mm_sub_epi32(lCpuData->Values[i], rCpuData->Values[i]);
			auto pow = _mm_mullo_epi32(diff, diff);

			_mm_storeu_si128((__m128i*)resultBuffer, pow);

			for (auto b = 0; b < bytesPerChannel; b++)
				result += static_cast<int64_t>(resultBuffer[b]);
		}

		return result / (double)(arrayLength * bytesPerChannel);
	}
};

IConvert* sseConvert = new SSEConvert();
IConvert* SSEBasedIConvert()
{
	return sseConvert;
}
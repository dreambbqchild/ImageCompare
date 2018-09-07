#include <immintrin.h>

void ZeroOutLowNibbleAVX(unsigned char* bytes, int length)
{
	static const __m256i ZERO_OUT_LOW_NIBBLE = _mm256_set1_epi16((short)0xF0F0);

	auto avxLength = length - (length % 32);
	for (auto i = 0; i < avxLength; i += 32)
	{
		auto ptr = &bytes[i];
		auto chunk = _mm256_loadu_si256((__m256i*)ptr);
		auto result = _mm256_and_si256(chunk, ZERO_OUT_LOW_NIBBLE);
		_mm256_storeu_si256((__m256i*)ptr, result);
	}

	for (auto i = avxLength; i < length; i++)
		bytes[i] &= 0xF0;
}

void ConvertBytesToShortsAVX(unsigned char* bytes, short* shorts, int length)
{
	static const auto zero = _mm256_setzero_si256();
	auto avxLength = length - (length % 32);
	for (auto i = 0; i < avxLength; i += 32)
	{
		auto ptr = &bytes[i];
		auto shortLo = &shorts[i];
		auto shortHi = &shorts[i + 16];
		auto chunk = _mm256_loadu_si256((__m256i*)ptr);
		auto lo = _mm256_unpacklo_epi8(chunk, zero);
		auto hi = _mm256_unpackhi_epi8(chunk, zero);

		_mm256_storeu_si256((__m256i*) shortLo, lo);
		_mm256_storeu_si256((__m256i*) shortHi, hi);
	}

	for (auto i = avxLength; i < length; i++)
		shorts[i] = bytes[i];
}

float CalcAverageDiffBetweenAVX(short* lShorts, short* rShorts, int length)
{
	static const __m256i zero = _mm256_setzero_si256();
	__declspec(align(32)) float alignedFloatBuffer[8] = { 0 };

	auto px = 1.0f;
	auto result = _mm256_setzero_ps();
	auto avxWidth = length - (length % 32);
	auto lPtr = lShorts;
	auto rPtr = rShorts;

	for (auto i = 0; i < avxWidth; i += 32, lPtr += 16, rPtr += 16)
	{				
		auto lChunk = _mm256_loadu_si256((__m256i*)lPtr);
		auto rChunk = _mm256_loadu_si256((__m256i*)rPtr);
		auto diff = _mm256_sub_epi16(lChunk, rChunk);
		diff = _mm256_abs_epi16(diff);

		auto low = _mm256_cvtepi32_ps(_mm256_unpacklo_epi16(diff, zero));
		auto hi = _mm256_cvtepi32_ps(_mm256_unpackhi_epi16(diff, zero));
						
		//Rolling Average
		for (auto k = 0; k < 2; k++)
		{
			auto n = _mm256_set1_ps(px++);
			auto current = _mm256_sub_ps(k == 0 ? low : hi, result);
			current = _mm256_div_ps(current, n);
			result = _mm256_add_ps(result, current);
		}
	}

	//Given the size of the images involved, the remaining pixels are being counted as insignificant.
	_mm256_store_ps(alignedFloatBuffer, result);

	for (auto i = 0; i < 3; i++)
		alignedFloatBuffer[i] = (alignedFloatBuffer[i] + alignedFloatBuffer[i + 3]) * 0.5f;

	return 1.0f - (((alignedFloatBuffer[0] + alignedFloatBuffer[1] + alignedFloatBuffer[2]) / 3.0f) / 255.0f);
}
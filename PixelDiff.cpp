#include "stdafx.h"
#include "PixelDiff.h"
#include "PixelDiffAVX.h"
#include <emmintrin.h>
#include <tmmintrin.h>

using namespace SIMD;

#pragma unmanaged

void ZeroOutLowNibbleSSE(unsigned char* bytes, int length)
{
	static const __m128i ZERO_OUT_LOW_NIBBLE = _mm_set1_epi16((short)0xF0F0);
	
	auto sseLength = length - (length % 16);
	for (auto i = 0; i < sseLength; i += 16)
	{
		auto ptr = &bytes[i];
		auto chunk = _mm_loadu_si128((__m128i*)ptr);
		auto result = _mm_and_si128(chunk, ZERO_OUT_LOW_NIBBLE);
		_mm_storeu_si128((__m128i*)ptr, result);
	}

	for (auto i = sseLength; i < length; i++)
		bytes[i] &= 0xF0;
}

void ConvertBytesToShortsSSE(unsigned char* bytes, short* shorts, int length)
{
	static const auto zero = _mm_setzero_si128();
	auto sseLength = length - (length % 16);
	for (auto i = 0; i < sseLength; i += 16)
	{
		auto ptr = &bytes[i];
		auto shortLo = &shorts[i];
		auto shortHi = &shorts[i + 8];
		auto chunk = _mm_loadu_si128((__m128i*)ptr);
		auto lo = _mm_unpacklo_epi8(chunk, zero);
		auto hi = _mm_unpackhi_epi8(chunk, zero);
		
		_mm_storeu_si128((__m128i*) shortLo, lo);
		_mm_storeu_si128((__m128i*) shortHi, hi);
	}

	for (auto i = sseLength; i < length; i++)
		shorts[i] = bytes[i];
}

float CalcAverageDiffBetweenSSE(short* lShorts, short* rShorts, int length)
{
	static const auto zero = _mm_setzero_si128();
	__declspec(align(16)) float alignedFloatBuffer[4] = { 0 };

	auto px = 1.0f;
	auto result = _mm_setzero_ps();
	auto sseWidth = length - (length % 16);
	auto lPtr = lShorts;
	auto rPtr = rShorts;

	for (auto i = 0; i < sseWidth; i += 16, lPtr += 8, rPtr += 8)
	{		
		auto lChunk = _mm_loadu_si128((__m128i*)lPtr);
		auto rChunk = _mm_loadu_si128((__m128i*)rPtr);
		auto diff = _mm_sub_epi8(lChunk, rChunk);
		diff = _mm_abs_epi16(diff);

		auto low = _mm_cvtepi32_ps(_mm_unpacklo_epi16(diff, zero));
		auto hi = _mm_cvtepi32_ps(_mm_unpackhi_epi16(diff, zero));

		//Rolling Average
		for (auto k = 0; k < 2; k++)
		{
			auto n = _mm_set_ps1(px++);
			auto current = _mm_sub_ps(k == 0 ? low : hi, result);
			current = _mm_div_ps(current, n);
			result = _mm_add_ps(result, current);
		}
	}

	//Given the size of the images involved, the remaining pixels are being counted as insignificant.
	_mm_store_ps(alignedFloatBuffer, result);
	return 1.0f - (((alignedFloatBuffer[0] + alignedFloatBuffer[1] + alignedFloatBuffer[2]) / 3.0f) / 255.0f);
}

#pragma managed

void PixelDiff::ZeroOutLowNibble(array<Byte>^ bytes)
{
	pin_ptr<unsigned char> ptr = &bytes[0];
	ZeroOutLowNibbleAVX(ptr, bytes->Length);
}

void PixelDiff::SetShorts(array<Byte>^ bytes)
{
	shorts = gcnew array<Int16>(bytes->Length);
	pin_ptr<unsigned char> ptrBytes = &bytes[0];
	pin_ptr<short> ptrShorts = &shorts[0];
	ConvertBytesToShortsAVX(ptrBytes, ptrShorts, bytes->Length);
}

float PixelDiff::CalcAverageDiffBetween(PixelDiff^ compareTo)
{
	if (shorts->Length != compareTo->shorts->Length)
		throw gcnew Exception();

	pin_ptr<short> lShort = &this->shorts[0];
	pin_ptr<short> rShort = &compareTo->shorts[0];
	return CalcAverageDiffBetweenAVX(lShort, rShort, shorts->Length);
}
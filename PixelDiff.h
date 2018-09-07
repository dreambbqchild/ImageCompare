#pragma once
using namespace System;
namespace SIMD
{
	public ref class PixelDiff
	{
	private:
		array<Int16>^ shorts;
		int width, height;
		void ZeroOutLowNibble(array<Byte>^ bytes);
		void SetShorts(array<Byte>^ bytes);

	public:
		PixelDiff(array<Byte>^ bytes, int width, int height) : width(width), height(height) { SetShorts(bytes); }
			
		property array<Int16>^ Buffer { array<Int16>^ get() { return shorts; } }
		property int Width { int get() { return width; } }
		property int Height { int get() { return height; } }

		float CalcAverageDiffBetween(PixelDiff^ compareTo);			
	};
}
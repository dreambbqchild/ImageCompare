#pragma once
#include "PixelDiffConvertContext.h"
#include "IConvert.h"
using namespace System;

public ref class PixelDiff
{
private:
	IConvert* converter;
	IConvertData* data;
	int32_t width, height, bytesPerChannel;
	void SetUnmanagedMemory(array<Byte>^ bytes);

public:
	PixelDiff(PixelDiffConvertContext^ context, array<Byte>^ bytes, int32_t width, int32_t height, int32_t bytesPerChannel);
	
	property int32_t Width { int get() { return width; } }
	property int32_t Height { int get() { return height; } }
	property int32_t BytesPerChannel { int get() { return bytesPerChannel; } }

	double CalcMeanSquaredError(PixelDiff^ compareTo);			

	virtual ~PixelDiff()
	{
		delete data;
	}
};
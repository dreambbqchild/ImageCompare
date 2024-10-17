#include "PixelDiff.h"
#include "IConvert.h"
#include <intrin.h>

PixelDiff::PixelDiff(PixelDiffConvertContext^ context, array<Byte>^ bytes, int32_t width, int32_t height, int32_t bytesPerChannel) 
	: width(width), height(height), bytesPerChannel(bytesPerChannel), data(nullptr)
{
	converter = context->GetConverter(width, height);
	SetUnmanagedMemory(bytes); 
}

void PixelDiff::SetUnmanagedMemory(array<Byte>^ bytes)
{
	pin_ptr<uint8_t> ptrBytes = &bytes[0];
	data = converter->PreflightData(ptrBytes, width, height, bytesPerChannel);
}

double PixelDiff::CalcMeanSquaredError(PixelDiff^ compareTo)
{	
	return converter->MeanSquaredError(data, compareTo->data);
}
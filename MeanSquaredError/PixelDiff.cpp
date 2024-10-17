#include "PixelDiff.h"
#include "IConvert.h"
#include <intrin.h>

PixelDiff::PixelDiff(PixelDiffConvertContext^ context, array<Byte>^ bytes, int32_t width, int32_t height) 
	: width(width), height(height), length(bytes->Length), data(nullptr)
{
	converter = context->GetConverter(width, height);
	SetShorts(bytes); 
}

void PixelDiff::SetShorts(array<Byte>^ bytes)
{
	pin_ptr<uint8_t> ptrBytes = &bytes[0];
	IConvertData* localData = nullptr;
	converter->PreflightData(ptrBytes, &localData, bytes->Length);
	data = localData;
}

float PixelDiff::CalcMeanSquaredError(PixelDiff^ compareTo)
{	
	return converter->MeanSquaredError(data, compareTo->data, length);
}
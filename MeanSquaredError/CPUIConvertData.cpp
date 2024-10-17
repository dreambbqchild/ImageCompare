#include "IConvert.h"

CPUIConvertData::CPUIConvertData(uint8_t* bytes, int32_t width, int32_t height, int32_t bytesPerChannel)
	: Width(width), Height(height), BytesPerChannel(bytesPerChannel)
{
	auto segmentSize = (width * height) / bytesPerChannel;
	Values = new uint8_t * [bytesPerChannel];

	for (auto i = 0; i < bytesPerChannel; i++)
	{
		Values[i] = new uint8_t[segmentSize];
	}
}

CPUIConvertData::~CPUIConvertData()
{
	for (auto i = 0; i < BytesPerChannel; i++)
		delete[] Values[i];

	delete[] Values;
}
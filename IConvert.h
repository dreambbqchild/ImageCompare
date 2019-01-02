#pragma once
#include <stdint.h>

class IConvertData
{
public:
	virtual ~IConvertData() {}
};

class CPUIConvertData : public IConvertData
{
public:		
	int16_t* Shorts;

	CPUIConvertData(int32_t length)
	{
		Shorts = new int16_t[length];
	}

	virtual ~CPUIConvertData()
	{
		delete[] Shorts;
	}
};

class IConvert
{
public:
	virtual void PreflightData(uint8_t* bytes, IConvertData** data, int32_t arrayLength) = 0;
	virtual float MeanSquaredError(IConvertData* lData, IConvertData* rData, int32_t arrayLength) = 0;
};

IConvert* AVXBasedIConvert();
IConvert* SSEBasedIConvert();
IConvert* ComputeShaderBasedIConvert(int32_t height, int32_t width);
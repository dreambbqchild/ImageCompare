#pragma once
#include <stdint.h>
#include <exception>
#include <tuple>

class IConvertData
{
public:
	virtual ~IConvertData() {}
};

template <typename T>
class CPUConvertData : public IConvertData
{
public:		
	T* Values;
	const int32_t ValuesLength, Width, Height, BytesPerChannel;

	CPUConvertData(size_t alignment, int32_t valuesLength, int32_t width, int32_t height, int32_t bytesPerChannel)
		: ValuesLength(valuesLength), Width(width), Height(height), BytesPerChannel(bytesPerChannel)
	{
		Values = static_cast<T*>(_aligned_malloc(valuesLength * sizeof(T), alignment));
	}

	virtual ~CPUConvertData()
	{
		_aligned_free(Values);
	}
};

#define VALIDATE_AND_EXTRACT(t, l, r, arrayLength, bytesPerChannel, lData, rData) \
t* l, *r; \
int32_t arrayLength, bytesPerChannel; \
std::tie(l, r, arrayLength, bytesPerChannel) = ValidateAndExtract<t>(lData, rData)

template <typename T>
std::tuple<T*, T*, int32_t, int32_t> ValidateAndExtract(IConvertData* lData, IConvertData* rData)
{
	auto l = static_cast<T*>(lData);
	auto r = static_cast<T*>(rData);

	auto arrayLength = l->ValuesLength == r->ValuesLength ? l->ValuesLength : throw new std::exception("ValuesLength must be the same");
	auto bytesPerChannel = l->BytesPerChannel == r->BytesPerChannel ? l->BytesPerChannel : throw new std::exception("BytesPerChannel must be the same");
	auto height = l->Height == r->Height ? l->Height : throw new std::exception("Height must be the same");
	auto width = l->Width == r->Width ? l->Width : throw new std::exception("Width must be the same");

	return {l, r, arrayLength, bytesPerChannel};
}

class IConvert
{
public:
	virtual IConvertData* PreflightData(uint8_t* bytes, int32_t width, int32_t height, int32_t bytesPerChannel) = 0;
	virtual double MeanSquaredError(IConvertData* lData, IConvertData* rData) = 0;
};

IConvert* CPUBasedIConvert();
IConvert* AVXBasedIConvert();
IConvert* SSEBasedIConvert();
IConvert* ComputeShaderBasedIConvert(int32_t height, int32_t width);
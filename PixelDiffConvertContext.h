#pragma once
#include <stdint.h>
#include <unordered_map>
class IConvert;

public enum class ConvertContext 
{
	Auto,
	AVX,
	SSE,
	ComputeShader
};

public ref class PixelDiffConvertContext
{
private:
	static PixelDiffConvertContext();
	std::unordered_map<uint64_t, IConvert*>* cache;
	IConvert* userDefinedConverter;
	ConvertContext convertContext;

internal:
	IConvert* GetConverter(int32_t width, int32_t height);

public:
	PixelDiffConvertContext();
	PixelDiffConvertContext(ConvertContext context);

	property ConvertContext Context
	{
		ConvertContext get() { return convertContext; }
	};

	~PixelDiffConvertContext();
};
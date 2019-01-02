#include "stdafx.h"
#include "PixelDiffConvertContext.h"
#include "IConvert.h"
#include "InstructionSet.h"

#define USE_COMPUTE_HARDWARE(milliseconds) milliseconds < 5

IConvert* cpuConverter = nullptr;
bool computeShaderChecked = false;
bool computeShaderSupported = true;
ConvertContext cpuContext;

static PixelDiffConvertContext::PixelDiffConvertContext()
{
	if (InstructionSet::AVX2())
	{
		cpuConverter = AVXBasedIConvert();
		cpuContext = ConvertContext::AVX;
	}
	else if (InstructionSet::SSE2())
	{
		cpuConverter = SSEBasedIConvert();
		cpuContext = ConvertContext::SSE;
	}
}

PixelDiffConvertContext::PixelDiffConvertContext() : userDefinedConverter(nullptr)
{
	cache = new std::unordered_map<uint64_t, IConvert*>();
	if (!computeShaderChecked)
	{
		auto shader = ComputeShaderBasedIConvert(1024, 768);
		computeShaderSupported = shader != nullptr;			

		if (computeShaderSupported)
		{
			auto const length = 1024 * 768 * 4;
			IConvertData* data;
			auto memory = static_cast<uint8_t*>(calloc(length, sizeof(uint8_t)));
			shader->PreflightData(memory, &data, length);
			//Run it once to basically finish initalization;
			shader->MeanSquaredError(data, data, length);

			//Now time it.
			auto timer = System::Diagnostics::Stopwatch::StartNew();			
			shader->MeanSquaredError(data, data, length);
			timer->Stop();

			//Some GPUs aren't worth the silicon they're on.
			//If we got one of those, force the use of the CPU cause even brute force will be faster.
			computeShaderSupported = USE_COMPUTE_HARDWARE(timer->ElapsedMilliseconds);

			free(memory);
			delete data;
			delete shader;
		}
		computeShaderChecked = true;
	}

	convertContext = computeShaderSupported ? ConvertContext::ComputeShader : cpuContext;
}

PixelDiffConvertContext::PixelDiffConvertContext(ConvertContext context) : convertContext(context)
{
	cache = new std::unordered_map<uint64_t, IConvert*>();
	
	switch (context)
	{	
	case ConvertContext::SSE:
		userDefinedConverter = SSEBasedIConvert();
		break;
	case ConvertContext::AVX:
		userDefinedConverter = AVXBasedIConvert();
		break;
	case ConvertContext::Auto:
	case ConvertContext::ComputeShader:		
	default:
		userDefinedConverter = nullptr;
		break;
	}
}

IConvert* PixelDiffConvertContext::GetConverter(int32_t width, int32_t height)
{
	if (userDefinedConverter)
		return userDefinedConverter;

	if (computeShaderSupported)
	{
		uint64_t key = 0;
		uint32_t* keyPtr = (uint32_t*)&key;
		keyPtr[1] = width;
		keyPtr[0] = height;

		auto result = (*cache)[key];
		if(result != nullptr)
			return result;

		result = ComputeShaderBasedIConvert(width, height);
		if (result)
		{
			(*cache)[key] = result;
			return result;
		}
		else
			computeShaderSupported = false;			
	}
	
	return cpuConverter;
}

PixelDiffConvertContext::~PixelDiffConvertContext()
{
	for (auto itr = cache->begin(); itr != cache->end(); itr++)
		delete itr->second;

	delete cache;
}

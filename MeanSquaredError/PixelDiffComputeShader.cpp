#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <stdint.h>
#include <atlbase.h>
#include <limits>
#include "IConvert.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3dcompiler.lib")

const int32_t X_THREADS = 16;
const int32_t OUTPUT_INTS = 1024;

class Shader 
{
private:
	CComPtr<ID3D11Device> pd3dDevice;
	CComPtr<ID3D11DeviceContext> pImmediateContext;
	CComPtr<ID3D11ComputeShader> shader;
	bool isValid;
	
	bool LoadShader()
	{
		const char* source = R"(StructuredBuffer<int> BufferLeft : register(t0);
StructuredBuffer<int> BufferRight : register(t1);
uint BufferLength : register(b0);
globallycoherent RWStructuredBuffer<int> BufferOut : register(u0);
[numthreads(16, 1, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
	uint index = threadID.x;
	if(index >= BufferLength)
		return;

	int lBytes = BufferLeft[index];
	int rBytes = BufferRight[index];
	int4 left = { lBytes & 0xff, (lBytes >> 8) & 0xff, (lBytes >> 16) & 0xff, (lBytes >> 24) & 0xff };
	int4 right = { rBytes & 0xff, (rBytes >> 8) & 0xff, (rBytes >> 16) & 0xff, (rBytes >> 24) & 0xff };
	int4 value = left - right;
	value = value * value;

	uint outIndex = index % 1024;

	InterlockedAdd(BufferOut[outIndex], value.x + value.y + value.z + value.w);
})";
		const auto length = strlen(source);

		CComPtr<ID3DBlob> pErrorBlob;
		CComPtr<ID3DBlob> pBlob;
		auto hr = D3DCompile(source, length, nullptr, nullptr, nullptr, "CSMain", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pBlob, &pErrorBlob);

		if (FAILED(hr))
		{
			if (pErrorBlob)
				OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

			return false;
		}

		hr = pd3dDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader);
		return SUCCEEDED(hr);
	}

public:
	Shader()
	{
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0
		};

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		isValid = SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 1, D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext));
		isValid = isValid && LoadShader();
		
		if(isValid)
			pImmediateContext->CSSetShader(shader, nullptr, 0);
	}

	bool GetIsValid() { return isValid; }
	inline CComPtr<ID3D11DeviceContext> GetImmediateContext() { return pImmediateContext; }

	bool CreateInputBuffer(void* data, uint32_t byteLength, CComPtr<ID3D11Buffer> &srcDataGPUBuffer, CComPtr<ID3D11ShaderResourceView> &srcDataGPUBufferView)
	{
		if (data && byteLength)
		{
			D3D11_BUFFER_DESC descGPUBuffer = {};
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			descGPUBuffer.ByteWidth = byteLength;
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = 4;

			D3D11_SUBRESOURCE_DATA InitData = {};
			InitData.pSysMem = data;
			if (FAILED(pd3dDevice->CreateBuffer(&descGPUBuffer, &InitData, &srcDataGPUBuffer)))
				return false;

			D3D11_BUFFER_DESC descBuf = {};
			srcDataGPUBuffer->GetDesc(&descBuf);

			D3D11_SHADER_RESOURCE_VIEW_DESC descView = {};
			descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			descView.Buffer.FirstElement = 0;

			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

			if (FAILED(pd3dDevice->CreateShaderResourceView(srcDataGPUBuffer, &descView, &srcDataGPUBufferView)))
				return false;

			return true;
		}
		else
			return false;
	}

	bool CreateOutputBuffer(uint32_t byteLength, CComPtr<ID3D11Buffer> &destDataGPUBuffer, CComPtr<ID3D11UnorderedAccessView> &destDataGPUBufferView)
	{
		D3D11_BUFFER_DESC descGPUBuffer = {};
		descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descGPUBuffer.ByteWidth = byteLength;
		descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descGPUBuffer.StructureByteStride = 4;

		if (FAILED(pd3dDevice->CreateBuffer(&descGPUBuffer, NULL, &destDataGPUBuffer)))
			return false;

		D3D11_BUFFER_DESC descBuf = {};
		destDataGPUBuffer->GetDesc(&descBuf);

		D3D11_UNORDERED_ACCESS_VIEW_DESC descView = { };
		descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		descView.Buffer.FirstElement = 0;

		descView.Format = DXGI_FORMAT_UNKNOWN;
		descView.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

		if (FAILED(pd3dDevice->CreateUnorderedAccessView(destDataGPUBuffer, &descView, &destDataGPUBufferView)))
			return false;

		return true;
	}

	template <typename T>
	bool CreateConstantBuffer(T data, CComPtr<ID3D11Buffer> &constantBuffer)
	{
		uint8_t buffer[16] = {}; // Constant buffers require a byte width that is an multiple of 16.
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(buffer);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		memcpy(buffer, &data, sizeof(T));

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = buffer;

		if (FAILED(pd3dDevice->CreateBuffer(&bufferDesc, &initData, &constantBuffer)))
			return false;

		return true;
	}

	bool ReadBufferDataInto(void* outBuff, uint32_t byteLength, CComPtr<ID3D11Buffer> &dataBuffer)
	{
		CComPtr<ID3D11Buffer> buffer;

		D3D11_BUFFER_DESC desc = {};
		dataBuffer->GetDesc(&desc);

		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;

		if (SUCCEEDED(pd3dDevice->CreateBuffer(&desc, nullptr, &buffer)))
		{
			pImmediateContext->CopyResource(buffer, dataBuffer);			

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (pImmediateContext->Map(buffer, 0, D3D11_MAP_READ, 0, &mappedResource) != S_OK)
				return false;

			memcpy(outBuff, mappedResource.pData, min(desc.ByteWidth, (uint32_t)byteLength));
			
			pImmediateContext->Unmap(buffer, 0);
			return true;
		}
		return false;
	}

	~Shader()
	{
		pd3dDevice = nullptr;
		pImmediateContext = nullptr;
		shader = nullptr;
	}
};

class ComputeShaderConvertData : public IConvertData
{
public:
	CComPtr<ID3D11Buffer> GPUBuffer;
	CComPtr<ID3D11ShaderResourceView> GPUBufferView;

	const int32_t Width, Height, BytesPerChannel;

	ComputeShaderConvertData(int32_t width, int32_t height, int32_t bytesPerChannel) : Width(width), Height(height), BytesPerChannel(bytesPerChannel) {}
};

class ComputeShaderBasedConvert : public IConvert
{
private:
	Shader mseShader;

	CComPtr<ID3D11Buffer> destGPUBuffer;
	CComPtr<ID3D11Buffer> clearGPUBuffer;
	CComPtr<ID3D11UnorderedAccessView> destGPUBufferView;
	CComPtr<ID3D11Buffer> constantBuffer;

	uint32_t resultBuffer[OUTPUT_INTS] = { 0 };

	bool isValid;

public:
	ComputeShaderBasedConvert(int32_t width, int32_t height)
	{
		CComPtr<ID3D11UnorderedAccessView> throwAway;

		auto bufferLengthAsInts = width * height;
		isValid = mseShader.GetIsValid();
		isValid = isValid && mseShader.CreateConstantBuffer(bufferLengthAsInts, constantBuffer);
		isValid = isValid && mseShader.CreateOutputBuffer(sizeof(resultBuffer), destGPUBuffer, destGPUBufferView);		
		isValid = isValid && mseShader.CreateOutputBuffer(sizeof(resultBuffer), clearGPUBuffer, throwAway);

		if (isValid)
		{
			auto pImmediateContext = mseShader.GetImmediateContext();
			pImmediateContext->CSSetConstantBuffers(0, 1, &constantBuffer.p);
			pImmediateContext->CSSetUnorderedAccessViews(0, 1, &destGPUBufferView.p, nullptr);
		}
	}

	bool GetIsValid() { return isValid; }

	IConvertData* PreflightData(uint8_t* bytes, int32_t width, int32_t height, int32_t bytesPerChannel)
	{
		if (!isValid || bytesPerChannel != 4)
			return nullptr;

		auto localData = new ComputeShaderConvertData(width, height, bytesPerChannel);
		isValid = mseShader.CreateInputBuffer(bytes, width * height * bytesPerChannel, localData->GPUBuffer, localData->GPUBufferView);
		return localData;
	}

	double MeanSquaredError(IConvertData* lData, IConvertData* rData)
	{
		if (!isValid)
			return std::numeric_limits<float>::infinity();

		int64_t sum = 0;				
		auto lShaderData = static_cast<ComputeShaderConvertData*>(lData);
		auto rShaderData = static_cast<ComputeShaderConvertData*>(rData);		

		auto pImmediateContext = mseShader.GetImmediateContext();

		ID3D11ShaderResourceView* views[2] = { lShaderData->GPUBufferView, rShaderData->GPUBufferView };
		pImmediateContext->CSSetShaderResources(0, 2, views);		
		
		auto widthGroups = (lShaderData->Width * lShaderData->Height) / X_THREADS + (lShaderData->Width % X_THREADS == 0 ? 0 : 1);
		pImmediateContext->Dispatch(widthGroups, 1, 1);
		mseShader.ReadBufferDataInto(resultBuffer, sizeof(resultBuffer), destGPUBuffer);
		pImmediateContext->CopyResource(destGPUBuffer, clearGPUBuffer);

		for(auto i = 0; i < OUTPUT_INTS; i++)
			sum += resultBuffer[i];

		return sum / (double)(lShaderData->Width * lShaderData->Height * lShaderData->BytesPerChannel);
	}
};

IConvert* ComputeShaderBasedIConvert(int32_t height, int32_t width)
{
	auto result = new ComputeShaderBasedConvert(height, width);
	if (result->GetIsValid())
		return result;

	delete result;
	return nullptr;
}
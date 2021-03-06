#include "resource.h"
#include <d3d11.h>
#include <directxmath.h>
#include <stdint.h>
#include <atlbase.h>
#include <limits>
#include "IConvert.h"

#pragma comment(lib, "d3d11.lib")

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

class Shader 
{
private:
	CComPtr<ID3D11Device> pd3dDevice;
	CComPtr<ID3D11DeviceContext> pImmediateContext;
	CComPtr<ID3D11ComputeShader> shader;
	bool isValid;
	
	bool LoadShader(LPCWSTR resource)
	{
		auto res = FindResource(HINST_THISCOMPONENT, resource, RT_RCDATA);
		if (!res)
			return false;
		
		auto hResource = LoadResource(HINST_THISCOMPONENT, res);
		if (!hResource)
			return false;

		auto bytecodeLength = SizeofResource(HINST_THISCOMPONENT, res);
		auto data = LockResource(hResource);

		auto hr = pd3dDevice->CreateComputeShader(data, bytecodeLength, nullptr, &shader);		
		FreeResource(hResource);

		return SUCCEEDED(hr);
	}

public:
	Shader(LPCWSTR resource)
	{
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0
		};

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		isValid = SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 1, D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext));
		isValid = isValid && LoadShader(resource);
		
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
			descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			descView.BufferEx.FirstElement = 0;

			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

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

	bool CreateConstantBuffer(void* data, uint32_t byteLength, CComPtr<ID3D11Buffer> &constantBuffer)
	{
		int8_t buffer[16] = {};
		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = 16;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		memcpy(buffer, data, byteLength);

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = buffer;

		if (FAILED(pd3dDevice->CreateBuffer(&cbDesc, &InitData, &constantBuffer)))
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

class ComputeShaderIConvertData : public IConvertData
{
public:
	CComPtr<ID3D11Buffer> GPUBuffer;
	CComPtr<ID3D11ShaderResourceView> GPUBufferView;
};

class ComputeShaderBasedConvert : public IConvert
{
private:
	Shader mseShader = Shader(MAKEINTRESOURCE(SHADER_RESOURCE));

	CComPtr<ID3D11Buffer> destGPUBuffer;
	CComPtr<ID3D11Buffer> clearGPUBuffer;
	CComPtr<ID3D11UnorderedAccessView> destGPUBufferView;
	CComPtr<ID3D11Buffer> constantBuffer;

	uint32_t resultBuffer[16] = { 0 };
	int32_t height, width, widthGroups, heightGroups;

	bool isValid;

public:
	ComputeShaderBasedConvert(int32_t height, int32_t width) : height(height), width(width)
	{
		CComPtr<ID3D11UnorderedAccessView> throwAway;

		isValid = mseShader.GetIsValid();
		isValid = isValid && mseShader.CreateConstantBuffer(&width, sizeof(int32_t), constantBuffer);
		isValid = isValid && mseShader.CreateOutputBuffer(sizeof(resultBuffer), destGPUBuffer, destGPUBufferView);		
		isValid = isValid && mseShader.CreateOutputBuffer(sizeof(resultBuffer), clearGPUBuffer, throwAway);

		widthGroups = width / 8 + (width % 8 == 0 ? 0 : 1);
		heightGroups = height / 8 + (height % 8 == 0 ? 0 : 1);

		if (isValid)
		{
			auto pImmediateContext = mseShader.GetImmediateContext();
			pImmediateContext->CSSetConstantBuffers(0, 1, &constantBuffer.p);
			pImmediateContext->CSSetUnorderedAccessViews(0, 1, &destGPUBufferView.p, nullptr);
		}
	}

	bool GetIsValid() { return isValid; }

	void PreflightData(uint8_t* bytes, IConvertData** data, int32_t arrayLength)
	{
		if (!isValid)
			return;

		auto localData = new ComputeShaderIConvertData();
		isValid = mseShader.CreateInputBuffer(bytes, arrayLength, localData->GPUBuffer, localData->GPUBufferView);
		*data = localData;
	}

	float MeanSquaredError(IConvertData* lData, IConvertData* rData, int32_t arrayLength)
	{
		if (!isValid)
			return std::numeric_limits<float>::infinity();

		int64_t sum = 0;				
		auto lShaderData = static_cast<ComputeShaderIConvertData*>(lData);
		auto rShaderData = static_cast<ComputeShaderIConvertData*>(rData);		

		auto pImmediateContext = mseShader.GetImmediateContext();

		ID3D11ShaderResourceView* views[2] = { lShaderData->GPUBufferView, rShaderData->GPUBufferView };
		pImmediateContext->CSSetShaderResources(0, 2, views);		
		
		pImmediateContext->Dispatch(widthGroups, heightGroups, 1);
		mseShader.ReadBufferDataInto(resultBuffer, sizeof(resultBuffer), destGPUBuffer);
		pImmediateContext->CopyResource(destGPUBuffer, clearGPUBuffer);

		//When optimizing at the level of processor tick resolution, unrolling loops happens.
		sum = resultBuffer[0];
		sum += resultBuffer[1];
		sum += resultBuffer[2];
		sum += resultBuffer[3];
		sum += resultBuffer[4];
		sum += resultBuffer[5];
		sum += resultBuffer[6];
		sum += resultBuffer[8];
		sum += resultBuffer[9];
		sum += resultBuffer[10];
		sum += resultBuffer[11];
		sum += resultBuffer[12];
		sum += resultBuffer[13];
		sum += resultBuffer[14];
		sum += resultBuffer[15];

		return sum / (float)arrayLength;
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
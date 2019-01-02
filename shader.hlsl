StructuredBuffer<int> BufferLeft : register(t0);
StructuredBuffer<int> BufferRight : register(t1);
int ImageWidth : register(b0);
globallycoherent RWStructuredBuffer<int> BufferOut : register(u0);
[numthreads(8, 8, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
	uint index = (threadID.x + threadID.y * ImageWidth);
	int lBytes = BufferLeft[index];
	int rBytes = BufferRight[index];
	int4 left = { lBytes & 0xff, (lBytes >> 8) & 0xff, (lBytes >> 16) & 0xff, (lBytes >> 24) & 0xff };
	int4 right = { rBytes & 0xff, (rBytes >> 8) & 0xff, (rBytes >> 16) & 0xff, (rBytes >> 24) & 0xff };
	int4 value = left - right;
	value = value * value;

	uint outIndex = (index * 4) % 16;

	InterlockedAdd(BufferOut[outIndex], value.x);
	InterlockedAdd(BufferOut[outIndex + 1], value.y);
	InterlockedAdd(BufferOut[outIndex + 2], value.z);
	InterlockedAdd(BufferOut[outIndex + 3], value.w);
};
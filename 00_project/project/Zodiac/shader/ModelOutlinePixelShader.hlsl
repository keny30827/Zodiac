#include "header/ModelOutlineDataSet.hlsli"

OutputRenderTarget main(OutputVSPS input)
{
	OutputRenderTarget result;
	result.final = float4(1.0f, 0.0f, 0.0f, 1.0f);
	result.objectInfo = float4(0.75f, 0.0f, 0.0f, 0.0f);
	return result;
}
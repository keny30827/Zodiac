#include "SSRDataSet.hlsli"

OutputRenderTarget main(OutputVSPS input)
{
	OutputRenderTarget output;
	output.color = texColor.Sample(psSamp, input.uv);
	output.objectInfo = float4(1.0f, 0.0f, 0.0f, 0.0f);
	return output;
}
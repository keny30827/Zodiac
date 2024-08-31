#include "SSRDataSet.hlsli"

OutputRenderTarget main(OutputVSPS input)
{
	OutputRenderTarget output;
	output.color = texColor.Sample(psSamp, input.uv);
	output.normal = (normalize(input.normal) + 1.0f) / 2.0f;
	output.objectInfo = float4(1.0f, 0.5f, 0.5f, 0.5f);
	output.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	return output;
}
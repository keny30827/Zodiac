#include "SSRDataSet.hlsli"

OutputRenderTarget main(OutputVSPS input)
{
	float4 normal = normalize(input.normal);
	normal.y *= -1.0f;

	OutputRenderTarget output;
	output.color = texColor.Sample(psSamp, input.uv);
	output.normal = (normal + 1.0f) / 2.0f;
	output.objectInfo = float4(1.0f, 0.5f, 0.5f, 0.5f);
	return output;
}
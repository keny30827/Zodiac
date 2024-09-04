#include "SSRDataSet.hlsli"

OutputRenderTarget main(OutputVSPS input)
{
	OutputRenderTarget output;

	// スクリーン座標からuvを計算する.
	float2 displayUV = float2(input.pos.x / screenParam.x, input.pos.y / screenParam.y);

	// UVと深度からワールド空間に移動させる.
	float4 basePos = float4(displayUV, input.pos.z, 1.0f);
	{
		// クリップ空間に.
		basePos.x = basePos.x * 2.0f - 1.0f;
		basePos.y = (basePos.y * 2.0f - 1.0f) * -1.0f;
		// ビュー空間.
		basePos = mul(projInv, basePos);
		basePos /= basePos.w;
		// ワールド空間.
		basePos = mul(viewInv, basePos);
	}

	// ワールド空間上で法線から反射方向を決める.
	float4 ray = float4(0.0f, 0.0f, 0.0f, 0.0f);
	{
		float4 dirEyeToPos = float4(basePos.xyz - eye.xyz, 0.0f);
		ray = normalize(reflect(dirEyeToPos, normalize(input.normal)));
	}

	// 反射方向にステップを踏ませて、そこの位置をスクリーン座標にして深度をチェック.
	// 何かの裏側に入っていたらとりあえずそこを使う.入ってないなら次のステップ.
	// 何回かステップを踏み、何にもヒットしなかったら物体色を出す.
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 pos = basePos;
	[unroll]
	for (int n = 0; n < 128; ++n) {
		pos = pos + (ray * 0.20f);
		// ビュー空間.
		float4 viewPos = mul(view, pos);
		// クリップ空間.
		float4 clipPos = mul(proj, viewPos);
		clipPos /= clipPos.w;
		// uv.
		float2 checkUV = float2((clipPos.x + 1.0f) * 0.5f, (clipPos.y * -1.0f + 1.0f) * 0.5f);
		// 深度.
		float checkDepth = depth.Sample(psSamp, checkUV);
		// 何かの裏に隠れてる.
		if (checkDepth < clipPos.z) {
			// 前面にある位置をワールド位置に戻して、Zの差分がでかすぎる場合は無視する.
			float4 frontPos = float4(checkUV, checkDepth, 1.0f);
			// クリップ空間に.
			frontPos.x = frontPos.x * 2.0f - 1.0f;
			frontPos.y = (frontPos.y * 2.0f - 1.0f) * -1.0f;
			// ビュー空間.
			frontPos = mul(projInv, frontPos);
			frontPos /= frontPos.w;
			// ワールド空間.
			frontPos = mul(viewInv, frontPos);
			if ((pos.z - frontPos.z) <= 2.75f) {
				color = texColor.Sample(psSamp, checkUV);
				break;
			}
		}
	}

	output.color = color;
	output.normal = (normalize(input.normal) + 1.0f) / 2.0f;
	output.objectInfo = float4(1.0f, 0.5f, 0.5f, 0.5f);
	output.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	return output;
}
#include "SSAODataSet.hlsli"

// 疑似乱数を生成できるとか.線形合同法か何かかと思うが、正式な原理はわからず.
float random(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float4 main(OutputVSPS input) : SV_TARGET
{
	// 今のピクセルの深度.
	float z = psDepthTex.Sample(psSamp, input.uv);

	// 深度と今のピクセルのUV値から、元の3D座標を復元する.
	// クリップ空間だと、x = -1.0f ~ 1.0f, y = -1.0f ~ 1.0f, z = 0.0f ~ 1.0fになるはずなので,.
	// UV値をXYに見立てて範囲を調整している.
	float3 clipPos = float3(input.uv * 2.0f - 1.0f, z);
	clipPos.y *= -1.0f;
	float4 threeDPos = mul(projInv, float4(clipPos, 1.0f));
	threeDPos /= threeDPos.w;

	// 今のピクセルの法線.
	float4 normal = psGBufNormal.Sample(psSamp, input.uv) * 2.0f - 1.0f;

	// 1/π ∫Ω　V(p, v) cosΘ dω.V()は適用率判定関数（true or false）.cosΘはランバート項.
	// 上は、1/π ∫0,2π ∫0,1/2π　V(p, v) cosΘ (rdΘ) (rsinΘdφ) とも書ける.球面積分.
	float radius = 0.5f;
	float ao = 0.0f;
	float ao_norm = 0.0f;
	if (z < 1.0f) {
		[unroll]
		for (int n = 0; n < 256; n++) {
			float rand_x = random(float2(n, n)) * 2.0f - 1.0f;
			float rand_y = random(float2(rand_x, n)) * 2.0f - 1.0f;
			float rand_z = random(float2(rand_x, rand_y)) * 2.0f - 1.0f;
			float4 omega = normalize(float4(rand_x, rand_y, rand_z, 0.0f));

			// 法線とランダム方向ベクトルとの内積をとって、半球におさまっているかの判定＆ランバートの結果として使う.
			float _dot = dot(omega, normal);

			// 符号を抜き出す.
			float unit = sign(_dot);
			if (unit < 0.0f) {
				// 半球に収めるために符号反転.
				omega *= unit;
				// ランバートで使うので一応計算し直し.
				_dot = dot(omega, normal);
			}

			// 飛ばしたベクトル分の総和は計算しておいて、最後に正規化目的で使う.
			// V(p, v) cosΘ の V(p, v) が 1.0f と想定した時の計算.
			ao_norm += _dot;

			// 今の位置とランダム方向ベクトル(長さはradius)から、チェックする位置を計算する.
			float4 checkPos = threeDPos + (omega * radius);
			float4 clipPos = mul(proj, checkPos);
			clipPos /= clipPos.w;

			// V(p, v) cosΘ.
			float2 uv = (clipPos.xy + 1.0f) * 0.5f;
			uv.y *= -1.0f;
			ao += step(psDepthTex.Sample(psSamp, uv), clipPos.z) * _dot;
		}
		if (ao_norm > 0.0f) {
			ao /= ao_norm;
		}
	}

	return (1.0f - ao);
}
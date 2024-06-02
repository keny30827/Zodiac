#include "header/SpriteDataSet.hlsli"

// ���].
float4 Inverse(float4 color)
{
	return float4(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, color.a);
}

// ���m�N��.
float4 Monochrome(float4 color)
{
	// PAL�K�i�ɂ̂��Ƃ���YUV�ւ̕ϊ�����.
	float Y = dot(color.rgb, float3(0.299, 0.587, 0.1114));
	return float4(Y, Y, Y, color.a);
}

// ���g��.
float4 Retrospective(float4 color)
{
	// ���g��.4�~���ɗ��Ƃ���Ƃ̂��Ƃ����A�Ȃ�����ŗ�����̂��c�H
	return float4(color.rgb - fmod(color.rgb, 0.25), color.a);
}

// �u���[.
float4 Blur(OutputVSPS input)
{
	// ����9�ߖT�i���g���܂ށj�̕��ω�.
	float w, h, levels = 0.0f;
	psTex.GetDimensions(0, w, h, levels);
	float dx = (1.0f / w) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float dy = (1.0f / h) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float4 lt = psTex.Sample(psSamp, input.uv + float2(-dx, -dy));
	float4 ct = psTex.Sample(psSamp, input.uv + float2(0.0f, -dy));
	float4 rt = psTex.Sample(psSamp, input.uv + float2(dx, -dy));
	float4 lc = psTex.Sample(psSamp, input.uv + float2(-dx, 0.0f));
	float4 cc = psTex.Sample(psSamp, input.uv);
	float4 rc = psTex.Sample(psSamp, input.uv + float2(dx, 0.0f));
	float4 lb = psTex.Sample(psSamp, input.uv + float2(-dx, dy));
	float4 cb = psTex.Sample(psSamp, input.uv + float2(0.0f, dy));
	float4 rb = psTex.Sample(psSamp, input.uv + float2(dx, dy));
	return ((lt + ct + rt + lc + cc + rc + lb + cb + rb) / 9.0f);
}

// �G���{�X.
float4 Emboss(OutputVSPS input)
{
	float w, h, levels = 0.0f;
	psTex.GetDimensions(0, w, h, levels);
	float dx = (1.0f / w) * 2.0f;
	float dy = (1.0f / h) * 2.0f;
	float4 lt = psTex.Sample(psSamp, input.uv + float2(-dx, -dy)) * 2.0f;
	float4 ct = psTex.Sample(psSamp, input.uv + float2(0.0f, -dy)) * 1.0f;
	float4 rt = psTex.Sample(psSamp, input.uv + float2(dx, -dy)) * 0.0f;
	float4 lc = psTex.Sample(psSamp, input.uv + float2(-dx, 0.0f)) * 1.0f;
	float4 cc = psTex.Sample(psSamp, input.uv) * 1.0f;
	float4 rc = psTex.Sample(psSamp, input.uv + float2(dx, 0.0f)) * -1.0f;
	float4 lb = psTex.Sample(psSamp, input.uv + float2(-dx, dy)) * 0.0f;
	float4 cb = psTex.Sample(psSamp, input.uv + float2(0.0f, dy)) * -1.0f;
	float4 rb = psTex.Sample(psSamp, input.uv + float2(dx, dy)) * -2.0f;
	return (lt + ct + rt + lc + cc + rc + lb + cb + rb);
}

// �V���[�v�l�X.
float4 Sharpness(OutputVSPS input)
{
	// ����9�ߖT�i���g���܂ށj�̕��ω�.
	float w, h, levels = 0.0f;
	psTex.GetDimensions(0, w, h, levels);
	float dx = (1.0f / w) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float dy = (1.0f / h) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float4 lt = psTex.Sample(psSamp, input.uv + float2(-dx, -dy)) * 0.0f;
	float4 ct = psTex.Sample(psSamp, input.uv + float2(0.0f, -dy)) * -1.0f;
	float4 rt = psTex.Sample(psSamp, input.uv + float2(dx, -dy)) * 0.0f;
	float4 lc = psTex.Sample(psSamp, input.uv + float2(-dx, 0.0f)) * -1.0f;
	float4 cc = psTex.Sample(psSamp, input.uv) * 5.0f;
	float4 rc = psTex.Sample(psSamp, input.uv + float2(dx, 0.0f)) * -1.0f;
	float4 lb = psTex.Sample(psSamp, input.uv + float2(-dx, dy)) * 0.0f;
	float4 cb = psTex.Sample(psSamp, input.uv + float2(0.0f, dy)) * -1.0f;
	float4 rb = psTex.Sample(psSamp, input.uv + float2(dx, dy)) * 0.0f;
	return (lt + ct + rt + lc + cc + rc + lb + cb + rb);
}

// �G�b�W���o.
float4 Edge(OutputVSPS input)
{
	// ����9�ߖT�i���g���܂ށj�̕��ω�.
	float w, h, levels = 0.0f;
	psTex.GetDimensions(0, w, h, levels);
	float dx = (1.0f / w) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float dy = (1.0f / h) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	// ���v�l���O�ɂ���悤��.
	float4 lt = psTex.Sample(psSamp, input.uv + float2(-dx, -dy)) * 0.0f;
	float4 ct = psTex.Sample(psSamp, input.uv + float2(0.0f, -dy)) * -1.0f;
	float4 rt = psTex.Sample(psSamp, input.uv + float2(dx, -dy)) * 0.0f;
	float4 lc = psTex.Sample(psSamp, input.uv + float2(-dx, 0.0f)) * -1.0f;
	float4 cc = psTex.Sample(psSamp, input.uv) * 4.0f;
	float4 rc = psTex.Sample(psSamp, input.uv + float2(dx, 0.0f)) * -1.0f;
	float4 lb = psTex.Sample(psSamp, input.uv + float2(-dx, dy)) * 0.0f;
	float4 cb = psTex.Sample(psSamp, input.uv + float2(0.0f, dy)) * -1.0f;
	float4 rb = psTex.Sample(psSamp, input.uv + float2(dx, dy)) * 0.0f;
	float4 ret = (lt + ct + rt + lc + cc + rc + lb + cb + rb);
	// ���m�N����.
	ret = Monochrome(ret);
	// ���]�����Ă݂₷��.
	ret = Inverse(ret);
	// �������߂Ă݂�.
	ret = pow(ret, 10.0f);
	// ������݂��ĕs�v�ȕ�������Ă݂�.
	ret = step(0.2f, ret);
	return ret;
}

// �K�E�V�A���u���[.
float4 GaussianBlur(OutputVSPS input)
{
	// ����9�ߖT�i���g���܂ށj�̕��ω�.
	float w, h, levels = 0.0f;
	psTex.GetDimensions(0, w, h, levels);
	float dx = (1.0f / w) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float dy = (1.0f / h) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float4 ret = 0.0f;
	// �����̈ʒu.
	{
		ret += psTex.Sample(psSamp, input.uv) * weight[0][0];
	}
	if (isEnableX > 0.0f) {		
		// �㉺.
		for (uint n = 1; n < 8; n++) {
			ret += psTex.Sample(psSamp, input.uv + float2(dx * float(n), 0.0f)) * weight[n >> 2][n % 4];
			ret += psTex.Sample(psSamp, input.uv + float2(-dx * float(n), 0.0f)) * weight[n >> 2][n % 4];
		}
	}
	if (isEnableY > 0.0f) {
		// ���E.
		for (uint n = 1; n < 8; n++) {
			ret += psTex.Sample(psSamp, input.uv + float2(0.0f, dy * float(n))) * weight[n >> 2][n % 4];
			ret += psTex.Sample(psSamp, input.uv + float2(0.0f, -dy * float(n))) * weight[n >> 2][n % 4];
		}
	}
	return ret;
}

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 ret = 0.0f;
	if ((isEnableX > 0.0f) || (isEnableY > 0.0f)) {
		ret = GaussianBlur(input);
	}
	else if (isEnableDof > 0.0f) {
		float base = psDepthTex.Sample(psSamp, float2(0.5f, 0.5f));
		float now = psDepthTex.Sample(psSamp, input.uv);
		float diffDepth = saturate(pow(abs(base - now), 0.6f)) * 8.0f;
		float nPos, fPos;
		fPos = modf(diffDepth, nPos);
		float4 color[2];
		if (nPos == 0) {
			color[0] = psTex.Sample(psSamp, input.uv);
			color[1] = psDof.Sample(psSamp, input.uv);
		}
		else if(nPos == 1){
			color[0] = psDof.Sample(psSamp, input.uv);
			color[1] = psDofShrink.Sample(psSamp, input.uv * 0.5f);
		}
		else {
			float2 sizeUV = input.uv * 0.5f;
			float2 posUV = float2(0.0f, 0.0f);
			for (int n = 2; n <= nPos; n++) {
				posUV.y = posUV.y + sizeUV.y;
				sizeUV = sizeUV * 0.5f;
			}
			color[0] = psDofShrink.Sample(psSamp, posUV + sizeUV);
			{
				posUV.y = posUV.y + sizeUV.y;
				sizeUV = sizeUV * 0.5f;
			}
			color[1] = psDofShrink.Sample(psSamp, posUV + sizeUV);
		}
		ret = lerp(color[0], color[1], fPos);
	}
	else {
#if 0
		// �@���擾���āA�c�݂�����.
		// RGB��0.0f~1.0f�A�@����-1.0f ~ 1.0f�Ȃ̂ŁA���낦��.
		float4 distortion = psNormalTex.Sample(psSamp, input.uv) * 2.0f - 1.0f;
		float4 color = psTex.Sample(psSamp, input.uv + float2(distortion.x * 0.1f, distortion.z * 0.1f));
#else
		float4 color = psTex.Sample(psSamp, input.uv);
#endif

		ret = color;
	}

	if (isEnableSsao > 0.0f) {
		ret *= psSSAO.Sample(psSamp, input.uv);
	}

	// TEST �[�x��`�悵�Ă݂�.
	// float4 depth = psDepthTex.Sample(psSamp, input.uv);
	// ret = float4(depth.r, depth.r, depth.r, depth.a);
	
	return ret;
}
#include "Motion.h"
#include <string>
#include <algorithm>

bool CMotion::Load(const char* fileName)
{
	if (!fileName) { return false; }
	
	FILE* fp = nullptr;
	fopen_s(&fp, fileName, "rb");
	if (!fp) { return false; }

	fread(&m_header, sizeof(m_header), 1, fp);

	uint32_t motionNum = 0;
	fread(&motionNum, sizeof(motionNum), 1, fp);

	for (int n = 0; n < static_cast<int>(motionNum); n++) {
		SMotion motion = {};
		motion.Serialize(fp);
		auto ret = m_motionMap.find(motion.boneName);
		if (ret != m_motionMap.end()) {
			m_motionMap[motion.boneName].push_back(motion);
		}
		else {
			std::vector<SMotion> list = {};
			list.push_back(motion);
			m_motionMap[motion.boneName] = list;
		}
		if (m_lastFlame < motion.flame) {
			m_lastFlame = static_cast<float>(motion.flame);
		}
	}

	// 各ボーンのデータは、フレームで整列させる.
	for (auto& it : m_motionMap) {
		std::sort(it.second.begin(), it.second.end(), [&](const auto& l, const auto& r) { return (l.flame < r.flame); });
	}

	fclose(fp);

	return true;
}

void CMotion::Update(CModel* model)
{
	if (!model) { return; }
	if (!m_isPlay) { return; }

	m_flame += 0.100f;

	// モーション適用.
	ApplyMotion(model);

	// 事前計算で求められているIK情報を適用する.
	ApplyIK(model);

	if (m_lastFlame <= m_flame) {
		if (m_isLoop) {
			m_flame = 0.0f;
		}
		else {
			m_isEnd = true;
		}
	}
}

void CMotion::PlayAnimation(bool isLoop)
{
	m_flame = 0.0f;
	m_isPlay = true;
	m_isEnd = false;
	m_isLoop = isLoop;
}

void CMotion::StopAnimation()
{
	m_isPlay = false;
}

void CMotion::ApplyMotion(CModel* model)
{
	if (!model) { return; }

	for (auto it : m_motionMap) {
		if (it.second.size() <= 0) { continue; }
		auto ret = std::find_if(it.second.rbegin(), it.second.rend(), [&](const auto r) { return (r.flame <= m_flame); });
		if (ret != it.second.rend()) {
			auto base = ret.base();
			if (base != it.second.end()) {
				// ベジェ曲線を使ったフレーム間の補間.
				float t = (m_flame - static_cast<float>(ret->flame)) / static_cast<float>(base->flame - ret->flame);
				t = GetYfromXOnBezier(t, *ret, 15);

				// 経過時間に合わせた回転行列計算.
#if 0
				DirectX::XMMATRIX mat = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionSlerp(
					DirectX::XMLoadFloat4(&ret->quat),
					DirectX::XMLoadFloat4(&base->quat),
					t));
#else
				DirectX::XMVECTOR srcQuat = DirectX::XMVectorScale(DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&ret->quat)), (1.0f - t));
				DirectX::XMVECTOR dstQuat = DirectX::XMVectorScale(DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&base->quat)), (t));
				DirectX::XMVECTOR resultQuat = DirectX::XMVectorAdd(srcQuat, dstQuat);
				DirectX::XMMATRIX rot = DirectX::XMMatrixRotationQuaternion(resultQuat);
#endif

				model->ApplyRotationBone(it.first, rot);

				// 経過時間に合わせた平行移動行列計算.
				DirectX::XMVECTOR srcTrans = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&ret->trans), (1.0f - t));
				DirectX::XMVECTOR dstTrans = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&base->trans), (t));
				DirectX::XMVECTOR resultTrans = DirectX::XMVectorAdd(srcTrans, dstTrans);
				model->ApplyTransBone(it.first, DirectX::XMMatrixTranslationFromVector(resultTrans));
			}
			else {
				model->ApplyRotationBone(it.first, DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&it.second[0].quat)));
				model->ApplyTransBone(it.first, DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&it.second[0].trans)));
			}

			// ボーンのモーション再生後の位置を計算.
			model->CalcAffineBone(it.first);
		}		
	}

	// モーション再生後の位置をルートから計算しておく.
	model->CalcAffineBoneRecursive("センター");
	model->ApplyAffineBoneRecursive("センター");
}

void CMotion::ApplyIK(CModel* model)
{
	if (!model) { return; }
	for (int n = 0; n < model->GetIkInfoNum(); n++) {
		const SIkInfo& ikInfo = model->GetIkInfo(n);
		switch (ikInfo.chainBoneNum) {
		case 0:
		{
			assert(0);
		}
		break;
		case 1:
		{
			// 間なしなのでIKというIKは不要.
			// 特定方向に向かせる処理行列を使う.
			SolveIK_LookAt(model, ikInfo);
		}
		break;
		case 2:
		{
			// 余弦定理を使ったIKで補正する.
			SolveIK_CosineFormura(model, ikInfo);
		}
		break;
		default:
		{
			// CCD-IKで補正する.
			SolveIK_CCD(model, ikInfo);
		}
		break;
		}
	}

	// モーション再生後の位置をルートから計算しておく.
	model->CalcAffineBoneRecursive("センター");
	model->ApplyAffineBoneRecursive("センター");
}

void CMotion::SolveIK_LookAt(CModel* model, const SIkInfo& ikInfo)
{
	if (!model) { return; }

	auto& src = model->GetRowBone(ikInfo.chainBone[0]);
	auto& end = model->GetRowBone(ikInfo.targetIdx);
	auto& dst = model->GetRowBone(ikInfo.boneIdx);

	DirectX::XMVECTOR srcMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&src.GetOriginPos()), src.GetRowAffine());
	DirectX::XMVECTOR endMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&end.GetOriginPos()), end.GetRowAffine());
	DirectX::XMVECTOR originVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(endMove, srcMove));

	DirectX::XMVECTOR dstMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&dst.GetOriginPos()), dst.GetRowAffine());
	DirectX::XMVECTOR targetVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(dstMove, srcMove));

	DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 right = { 1.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);
	DirectX::XMVECTOR rightVec = DirectX::XMLoadFloat3(&right);
	model->ApplyRotationBone(ikInfo.chainBone[0], LookAtMatrix(originVec, targetVec, upVec, rightVec));
	model->CalcAffineBoneRecursive(src.GetName());
	model->ApplyAffineBoneRecursive(src.GetName());
}

void CMotion::SolveIK_CosineFormura(CModel* model, const SIkInfo& ikInfo)
{
	if (!model) { return; }

	// C^2 = A^2 + B^2 - 2ABcos （余弦定理）を使う.
	// 式変形して、cos = (A^2 + B^2 - C^2) / 2AB として角度を求める.
	
	auto& root = model->GetRowBone(ikInfo.chainBone[1]);
	auto& middle = model->GetRowBone(ikInfo.chainBone[0]);
	auto& end = model->GetRowBone(ikInfo.targetIdx);
	auto& target = model->GetRowBone(ikInfo.boneIdx);

	// もともとの位置関係.
	DirectX::XMVECTOR rootToEnd_origin = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&end.GetOriginPos()), DirectX::XMLoadFloat3(&root.GetOriginPos()));
	DirectX::XMVECTOR middleToEnd_origin = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&end.GetOriginPos()), DirectX::XMLoadFloat3(&middle.GetOriginPos()));
	DirectX::XMVECTOR rootToMiddle_origin = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&middle.GetOriginPos()), DirectX::XMLoadFloat3(&root.GetOriginPos()));

	// モーション諸々適用後の位置関係.
	DirectX::XMVECTOR rootMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&root.GetOriginPos()), root.GetRowAffine());
	DirectX::XMVECTOR middleMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&middle.GetOriginPos()), middle.GetRowAffine());
	DirectX::XMVECTOR endMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&end.GetOriginPos()), end.GetRowAffine());
	DirectX::XMVECTOR targetMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&target.GetOriginPos()), target.GetRowAffine());
	DirectX::XMVECTOR rootToEnd = DirectX::XMVectorSubtract(endMove, rootMove);
	DirectX::XMVECTOR rootToTarget = DirectX::XMVectorSubtract(targetMove, rootMove);
	DirectX::XMVECTOR middleToEnd = DirectX::XMVectorSubtract(endMove, middleMove);
	DirectX::XMVECTOR rootToMiddle = DirectX::XMVectorSubtract(middleMove, rootMove);

	// どうせターゲットまで移動することになるので、ターゲットまでの距離と、各ボーンをつなぐ長さを使って、中間点の位置を決める.
	const float A = DirectX::XMVector3Length(rootToTarget).m128_f32[0];
	const float B = DirectX::XMVector3Length(rootToMiddle_origin).m128_f32[0];
	const float C = DirectX::XMVector3Length(middleToEnd_origin).m128_f32[0];

	const float thetaAB = acosf(((A * A) + (B * B) - (C * C)) / (2 * A * B));
	const float thetaBC = acosf(((B * B) + (C * C) - (A * A)) / (2 * B * C));

	// 回転軸を決める.
	DirectX::XMVECTOR axis = {};
	if (strcmp(middle.GetName(), "ひざ") == 0) {
		axis = DirectX::XMVector3Cross(DirectX::XMVector3Normalize(rootToTarget), DirectX::XMVector3Normalize(rootToEnd_origin));
	}
	else {
		axis = { 1.0f, 0.0f, 0.0f };
	}

	if (DirectX::XMVector3IsNaN(axis)) {
		return;
	}
	if (DirectX::XMVector3Length(axis).m128_f32[0] <= 0.0f) {
		return;
	}

	// ルート用.
	DirectX::XMMATRIX rootRot = DirectX::XMMatrixRotationAxis(axis, thetaAB);
	// 中間点用.
	DirectX::XMMATRIX middleRot = DirectX::XMMatrixRotationAxis(axis, thetaBC - DirectX::XM_PI);

	// 回転.
	model->ApplyRotationBone(ikInfo.chainBone[1], rootRot);
	model->ApplyRotationBone(ikInfo.chainBone[0], middleRot);
	model->CalcAffineBoneRecursive(root.GetName());
	model->ApplyAffineBoneRecursive(root.GetName());
}

void CMotion::SolveIK_CCD(CModel* model, const SIkInfo& ikInfo)
{
	if (!model) { return; }

	auto& end = model->GetRowBone(ikInfo.targetIdx);
	auto& target = model->GetRowBone(ikInfo.boneIdx);

	DirectX::XMVECTOR targetPos = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&target.GetOriginPos()), target.GetRowAffine());

	for (int n = 0; n < ikInfo.iterationNum; n++) {
		// 末端から順に回す.
		for (auto& it : ikInfo.chainBone) {
			// 末端がターゲットに限りなく近くなっているなら終了.
			DirectX::XMVECTOR endPos = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&end.GetOriginPos()), end.GetRowAffine());
			if (abs(DirectX::XMVector3Length(DirectX::XMVectorSubtract(endPos, targetPos)).m128_f32[0]) <= 0.001) {
				break;
			}

			// 末端までのベクトルと、ターゲットまでのベクトルを用意.
			auto& now = model->GetRowBone(it);
			DirectX::XMVECTOR nowPos = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&now.GetOriginPos()), now.GetRowAffine());
			DirectX::XMVECTOR nowToEnd = DirectX::XMVectorSubtract(endPos, nowPos);
			DirectX::XMVECTOR nowToTarget = DirectX::XMVectorSubtract(targetPos, nowPos);

			// 同じ方向だったら、ここでの回転は無視.
			DirectX::XMVECTOR rotAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVector3Normalize(nowToEnd), DirectX::XMVector3Normalize(nowToTarget)));
			if (DirectX::XMVector3IsNaN(rotAxis)) {
				continue;
			}
			if (DirectX::XMVector3Length(rotAxis).m128_f32[0] <= 0.0f) {
				continue;
			}

			// 末端までのベクトルと、ターゲットまでのベクトルから向きを計算.
			float angle = DirectX::XMVector3AngleBetweenVectors(nowToEnd, nowToTarget).m128_f32[0];
			angle = min(angle, ikInfo.rotateLimit);
			DirectX::XMMATRIX rootRot = DirectX::XMMatrixRotationAxis(rotAxis, angle);

			// 回転.
			model->ApplyRotationBone(it, rootRot);
			model->CalcAffineBoneRecursive(now.GetName());
			model->ApplyAffineBoneRecursive(now.GetName());
		}
	}
}

DirectX::XMMATRIX CMotion::LookAtMatrix(DirectX::XMVECTOR src, DirectX::XMVECTOR dst, DirectX::XMVECTOR up, DirectX::XMVECTOR right)
{
	// 一度向きを原点に戻してから、向かせたい方向に向ける.
	// srcに回転情報以外が入る場合は、inverseを使うように変える.回転だけなら転置でOK.
	return DirectX::XMMatrixTranspose(LookAtMatrix(src, up, right)) * LookAtMatrix(dst, up, right);
}

DirectX::XMMATRIX CMotion::LookAtMatrix(DirectX::XMVECTOR dst, DirectX::XMVECTOR up, DirectX::XMVECTOR right)
{
	DirectX::XMVECTOR Axis1st = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVector3Normalize(dst), DirectX::XMVector3Normalize(right)));
	DirectX::XMVECTOR Axis2nd = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(Axis1st, DirectX::XMVector3Normalize(dst)));
	if (abs(DirectX::XMVector3Dot(right, Axis2nd).m128_f32[0]) >= 1.0f) {
		Axis1st = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVector3Normalize(dst), DirectX::XMVector3Normalize(up)));
		Axis2nd = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(Axis1st, DirectX::XMVector3Normalize(dst)));
	}
	DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
	mat.r[0] = Axis1st;
	mat.r[1] = Axis2nd;
	mat.r[2] = dst;
	return mat;
}

// ３次ベジェ曲線.
// (1 - t)^3 * p0 + (1 - t)^2 * t * p1 + (1 - t) * t^2 * p2 + t^3 * p3.
float CMotion::GetYfromXOnBezier(const float x, const SMotion& data, const int approximateNum)
{
	const float rx1 = static_cast<float>(data.bezierParam[3]) / 127.0f;
	const float ry1 = static_cast<float>(data.bezierParam[7]) / 127.0f;
	const float rx2 = static_cast<float>(data.bezierParam[11]) / 127.0f;
	const float ry2 = static_cast<float>(data.bezierParam[15]) / 127.0f;

	// コントロールポイントの配置的に線形補間なので、そのまま返してよし.
	if ((rx1 == ry1) && (rx2 == ry2)) {
		return x;
	}

	// フレームをxにマッピングする.なので、xはステップですすむものとして仮定する.
	float t = x;

	// で、f(t) = xとなるような、f(t) = 3次ベジェ曲線に関して、ニュートン法を使って、出力がxとなるようなtの値を探す（近似する）.
	// f(t)はベジェ曲線の公式のpをxに置き換えた上で、tについてまとめた式.

	// 各tに関する次数に掛け合わせる係数.
	const float k0 = 1.0f + 3.0f * rx1 - 3.0f * rx2;
	const float k1 = 3.0f * rx2 - 6.0f * rx1;
	const float k2 = 3.0f * rx1;

	constexpr float eps = 0.0005f;
	for (int n = 0; n < approximateNum; n++) {
		const float ft = k0 * t * t * t + k1 * t * t + k2 * t;
		const float ret = ft - x;
		if (abs(ret) <= eps) {
			break;
		}
		const float ft_dash = 3.0f * k0 * t * t + 2.0f * k1 * t + k2;
		const float newton = t + ((x - ft) / ft_dash);
		t = newton;
	}


	// で、最後にf(t) = yとなるようなベジェ曲線にtを代入して、yを求めたらそれが答え.
	// (1 - t)^3 + (1 - t)^2 * t + (1 - t) * t^2 + t^3 を逆から並べてる.
	// なお、MMDのコントロールポイントはp0が(0,0)、p1が(1,1)なので、(1 - t)^3に相当する処理は割愛されてる.
	const float r = (1.0f - t);
	return t * t * t + 3.0f * t * t * r * ry2 + 3.0f * t * r * r * ry1;
}
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

	// �e�{�[���̃f�[�^�́A�t���[���Ő��񂳂���.
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

	// ���[�V�����K�p.
	ApplyMotion(model);

	// ���O�v�Z�ŋ��߂��Ă���IK����K�p����.
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
				// �x�W�F�Ȑ����g�����t���[���Ԃ̕��.
				float t = (m_flame - static_cast<float>(ret->flame)) / static_cast<float>(base->flame - ret->flame);
				t = GetYfromXOnBezier(t, *ret, 15);

				// �o�ߎ��Ԃɍ��킹����]�s��v�Z.
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

				// �o�ߎ��Ԃɍ��킹�����s�ړ��s��v�Z.
				DirectX::XMVECTOR srcTrans = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&ret->trans), (1.0f - t));
				DirectX::XMVECTOR dstTrans = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&base->trans), (t));
				DirectX::XMVECTOR resultTrans = DirectX::XMVectorAdd(srcTrans, dstTrans);
				model->ApplyTransBone(it.first, DirectX::XMMatrixTranslationFromVector(resultTrans));
			}
			else {
				model->ApplyRotationBone(it.first, DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&it.second[0].quat)));
				model->ApplyTransBone(it.first, DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&it.second[0].trans)));
			}

			// �{�[���̃��[�V�����Đ���̈ʒu���v�Z.
			model->CalcAffineBone(it.first);
		}		
	}

	// ���[�V�����Đ���̈ʒu�����[�g����v�Z���Ă���.
	model->CalcAffineBoneRecursive("�Z���^�[");
	model->ApplyAffineBoneRecursive("�Z���^�[");
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
			// �ԂȂ��Ȃ̂�IK�Ƃ���IK�͕s�v.
			// ��������Ɍ������鏈���s����g��.
			SolveIK_LookAt(model, ikInfo);
		}
		break;
		case 2:
		{
			// �]���藝���g����IK�ŕ␳����.
			SolveIK_CosineFormura(model, ikInfo);
		}
		break;
		default:
		{
			// CCD-IK�ŕ␳����.
			SolveIK_CCD(model, ikInfo);
		}
		break;
		}
	}

	// ���[�V�����Đ���̈ʒu�����[�g����v�Z���Ă���.
	model->CalcAffineBoneRecursive("�Z���^�[");
	model->ApplyAffineBoneRecursive("�Z���^�[");
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

	// C^2 = A^2 + B^2 - 2ABcos �i�]���藝�j���g��.
	// ���ό`���āAcos = (A^2 + B^2 - C^2) / 2AB �Ƃ��Ċp�x�����߂�.
	
	auto& root = model->GetRowBone(ikInfo.chainBone[1]);
	auto& middle = model->GetRowBone(ikInfo.chainBone[0]);
	auto& end = model->GetRowBone(ikInfo.targetIdx);
	auto& target = model->GetRowBone(ikInfo.boneIdx);

	// ���Ƃ��Ƃ̈ʒu�֌W.
	DirectX::XMVECTOR rootToEnd_origin = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&end.GetOriginPos()), DirectX::XMLoadFloat3(&root.GetOriginPos()));
	DirectX::XMVECTOR middleToEnd_origin = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&end.GetOriginPos()), DirectX::XMLoadFloat3(&middle.GetOriginPos()));
	DirectX::XMVECTOR rootToMiddle_origin = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&middle.GetOriginPos()), DirectX::XMLoadFloat3(&root.GetOriginPos()));

	// ���[�V�������X�K�p��̈ʒu�֌W.
	DirectX::XMVECTOR rootMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&root.GetOriginPos()), root.GetRowAffine());
	DirectX::XMVECTOR middleMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&middle.GetOriginPos()), middle.GetRowAffine());
	DirectX::XMVECTOR endMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&end.GetOriginPos()), end.GetRowAffine());
	DirectX::XMVECTOR targetMove = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&target.GetOriginPos()), target.GetRowAffine());
	DirectX::XMVECTOR rootToEnd = DirectX::XMVectorSubtract(endMove, rootMove);
	DirectX::XMVECTOR rootToTarget = DirectX::XMVectorSubtract(targetMove, rootMove);
	DirectX::XMVECTOR middleToEnd = DirectX::XMVectorSubtract(endMove, middleMove);
	DirectX::XMVECTOR rootToMiddle = DirectX::XMVectorSubtract(middleMove, rootMove);

	// �ǂ����^�[�Q�b�g�܂ňړ����邱�ƂɂȂ�̂ŁA�^�[�Q�b�g�܂ł̋����ƁA�e�{�[�����Ȃ��������g���āA���ԓ_�̈ʒu�����߂�.
	const float A = DirectX::XMVector3Length(rootToTarget).m128_f32[0];
	const float B = DirectX::XMVector3Length(rootToMiddle_origin).m128_f32[0];
	const float C = DirectX::XMVector3Length(middleToEnd_origin).m128_f32[0];

	const float thetaAB = acosf(((A * A) + (B * B) - (C * C)) / (2 * A * B));
	const float thetaBC = acosf(((B * B) + (C * C) - (A * A)) / (2 * B * C));

	// ��]�������߂�.
	DirectX::XMVECTOR axis = {};
	if (strcmp(middle.GetName(), "�Ђ�") == 0) {
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

	// ���[�g�p.
	DirectX::XMMATRIX rootRot = DirectX::XMMatrixRotationAxis(axis, thetaAB);
	// ���ԓ_�p.
	DirectX::XMMATRIX middleRot = DirectX::XMMatrixRotationAxis(axis, thetaBC - DirectX::XM_PI);

	// ��].
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
		// ���[���珇�ɉ�.
		for (auto& it : ikInfo.chainBone) {
			// ���[���^�[�Q�b�g�Ɍ���Ȃ��߂��Ȃ��Ă���Ȃ�I��.
			DirectX::XMVECTOR endPos = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&end.GetOriginPos()), end.GetRowAffine());
			if (abs(DirectX::XMVector3Length(DirectX::XMVectorSubtract(endPos, targetPos)).m128_f32[0]) <= 0.001) {
				break;
			}

			// ���[�܂ł̃x�N�g���ƁA�^�[�Q�b�g�܂ł̃x�N�g����p��.
			auto& now = model->GetRowBone(it);
			DirectX::XMVECTOR nowPos = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&now.GetOriginPos()), now.GetRowAffine());
			DirectX::XMVECTOR nowToEnd = DirectX::XMVectorSubtract(endPos, nowPos);
			DirectX::XMVECTOR nowToTarget = DirectX::XMVectorSubtract(targetPos, nowPos);

			// ����������������A�����ł̉�]�͖���.
			DirectX::XMVECTOR rotAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVector3Normalize(nowToEnd), DirectX::XMVector3Normalize(nowToTarget)));
			if (DirectX::XMVector3IsNaN(rotAxis)) {
				continue;
			}
			if (DirectX::XMVector3Length(rotAxis).m128_f32[0] <= 0.0f) {
				continue;
			}

			// ���[�܂ł̃x�N�g���ƁA�^�[�Q�b�g�܂ł̃x�N�g������������v�Z.
			float angle = DirectX::XMVector3AngleBetweenVectors(nowToEnd, nowToTarget).m128_f32[0];
			angle = min(angle, ikInfo.rotateLimit);
			DirectX::XMMATRIX rootRot = DirectX::XMMatrixRotationAxis(rotAxis, angle);

			// ��].
			model->ApplyRotationBone(it, rootRot);
			model->CalcAffineBoneRecursive(now.GetName());
			model->ApplyAffineBoneRecursive(now.GetName());
		}
	}
}

DirectX::XMMATRIX CMotion::LookAtMatrix(DirectX::XMVECTOR src, DirectX::XMVECTOR dst, DirectX::XMVECTOR up, DirectX::XMVECTOR right)
{
	// ��x���������_�ɖ߂��Ă���A���������������Ɍ�����.
	// src�ɉ�]���ȊO������ꍇ�́Ainverse���g���悤�ɕς���.��]�����Ȃ�]�u��OK.
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

// �R���x�W�F�Ȑ�.
// (1 - t)^3 * p0 + (1 - t)^2 * t * p1 + (1 - t) * t^2 * p2 + t^3 * p3.
float CMotion::GetYfromXOnBezier(const float x, const SMotion& data, const int approximateNum)
{
	const float rx1 = static_cast<float>(data.bezierParam[3]) / 127.0f;
	const float ry1 = static_cast<float>(data.bezierParam[7]) / 127.0f;
	const float rx2 = static_cast<float>(data.bezierParam[11]) / 127.0f;
	const float ry2 = static_cast<float>(data.bezierParam[15]) / 127.0f;

	// �R���g���[���|�C���g�̔z�u�I�ɐ��`��ԂȂ̂ŁA���̂܂ܕԂ��Ă悵.
	if ((rx1 == ry1) && (rx2 == ry2)) {
		return x;
	}

	// �t���[����x�Ƀ}�b�s���O����.�Ȃ̂ŁAx�̓X�e�b�v�ł����ނ��̂Ƃ��ĉ��肷��.
	float t = x;

	// �ŁAf(t) = x�ƂȂ�悤�ȁAf(t) = 3���x�W�F�Ȑ��Ɋւ��āA�j���[�g���@���g���āA�o�͂�x�ƂȂ�悤��t�̒l��T���i�ߎ�����j.
	// f(t)�̓x�W�F�Ȑ��̌�����p��x�ɒu����������ŁAt�ɂ��Ă܂Ƃ߂���.

	// �et�Ɋւ��鎟���Ɋ|�����킹��W��.
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


	// �ŁA�Ō��f(t) = y�ƂȂ�悤�ȃx�W�F�Ȑ���t�������āAy�����߂��炻�ꂪ����.
	// (1 - t)^3 + (1 - t)^2 * t + (1 - t) * t^2 + t^3 ���t������ׂĂ�.
	// �Ȃ��AMMD�̃R���g���[���|�C���g��p0��(0,0)�Ap1��(1,1)�Ȃ̂ŁA(1 - t)^3�ɑ������鏈���͊�������Ă�.
	const float r = (1.0f - t);
	return t * t * t + 3.0f * t * t * r * ry2 + 3.0f * t * r * r * ry1;
}
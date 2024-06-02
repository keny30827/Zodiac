#include "Bone.h"

bool CBone::Load(const SMMDBone& readData)
{
	memcpy(m_name, readData.name, sizeof(m_name));
	m_parentNo = readData.parentNo;
	m_nextNo = readData.nextNo;
	m_type = readData.type;
	m_ikBoneNo = readData.ikBoneNo;
	m_pos = readData.pos;
	return true;
}

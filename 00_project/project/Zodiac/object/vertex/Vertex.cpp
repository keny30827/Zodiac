#include "Vertex.h"

bool CVertex::Load(const SMMDVertex& readData)
{
	m_position = readData.position;
	m_normal = readData.normal;
	m_uv = readData.uv;
	memcpy(m_boneNo, readData.boneNo, sizeof(m_boneNo));
	m_boneWeight = readData.boneWeight;
	m_edgeOption = readData.edgeOption;
	return true;
}

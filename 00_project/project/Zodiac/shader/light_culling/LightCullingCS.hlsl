#include "LightCullingDataSet.hlsli"


[numthreads(LIGHT_TILE_WIDTH, LIGHT_TILE_HEIGHT, 1)]
void main(
	uint3 groupId          : SV_GroupID,
	uint3 dispatchThreadId : SV_DispatchThreadID,
	uint3 groupThreadId : SV_GroupThreadID
)
{
	rwLightIndices[0] = 0;
}
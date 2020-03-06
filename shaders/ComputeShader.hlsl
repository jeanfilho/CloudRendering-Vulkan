#pragma shader_stage(compute)

RWTexture2D<float> renderImage;

cbuffer CameraProperties
{
	float3 position;
	float3 direction;
	uint width, height;
};

RWTexture3D<float> cloudGrid;
cbuffer CloudGridProperties
{
	float3 origin; // position of vtx (0,0,0)
	float3 size; // total size of the grid, calculated from voxelCount and voxelSize
	uint3 voxelCount; // number of samples in each dimension
	float3 voxelSize; // distance between samples
};

[numthreads(groupDimX, groupDimY, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	renderImage[DTid.xy] = 0.5f;
}
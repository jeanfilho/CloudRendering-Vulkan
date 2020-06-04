#pragma once

struct CloudProperties;
struct ShadowVolumeProperties;
namespace tests
{
	void RunTests();

	void createOrthonormalBasis(const glm::vec3& dir, glm::vec3& t0, glm::vec3& t1);

	struct Ray
	{
		glm::vec3 pos;
		glm::vec3 dir;
	};

	int isNegativeSign(float value);

	bool intersectCloud(CloudProperties& cloudProperties, glm::vec3& rayDir, glm::vec3& rayPos, glm::vec3& intersectionPoint);

	bool shadowVolumeTest(glm::uvec2 columnCoord, ShadowVolumeProperties& shadowVolumeProperties, const CloudProperties& cloudProperties);

	bool isInCloud(glm::vec3 point, const CloudProperties& cloudProperties);

	glm::vec3 calculateVoxelPosition(glm::uvec3 voxelIdx, ShadowVolumeProperties& shadowVolumeProperties);

	void localSort();

	void radixSort(std::vector<unsigned int>& keys, std::vector<unsigned int>& scatterOffsets, unsigned int nthShift);
}

#include "stdafx.h"
#include "Tests.h"

#include "UniformBuffers.h"
#include "Grid3D.h"

#include<random>

void tests::RunTests()
{
	localSort();

	bool test = true;
}

void tests::createOrthonormalBasis(const glm::vec3& dir, glm::vec3& t0, glm::vec3& t1)
{
	float a = dir.y / (1.0f + dir.z);
	float b = dir.y * a;
	float c = -dir.x * a;

	t0 = glm::vec3(dir.z + b, c, -dir.x);
	t1 = glm::vec3(c, 1.0f - b, -dir.y);
}

int tests::isNegativeSign(float value)
{
	return int(value < 0);
}

bool tests::intersectCloud(CloudProperties& cloudProperties, glm::vec3& rayDir, glm::vec3& rayPos, glm::vec3& intersectionPoint)
{
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	glm::vec3 invdir = 1.0f / rayDir;
	int sign[3] = { isNegativeSign(invdir.x), isNegativeSign(invdir.y), isNegativeSign(invdir.z) };

	intersectionPoint = glm::vec3(FLT_MAX);

	tmin = (cloudProperties.bounds[sign[0]].x - rayPos.x) * invdir.x;
	tmax = (cloudProperties.bounds[1 - sign[0]].x - rayPos.x) * invdir.x;
	tymin = (cloudProperties.bounds[sign[1]].y - rayPos.y) * invdir.y;
	tymax = (cloudProperties.bounds[1 - sign[1]].y - rayPos.y) * invdir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	tzmin = (cloudProperties.bounds[sign[2]].z - rayPos.z) * invdir.z;
	tzmax = (cloudProperties.bounds[1 - sign[2]].z - rayPos.z) * invdir.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	// We just want intersections in front of the ray
	if (tmax < 0) // ray outside of the cloud, pointing away
	{
		return false;
	}
	else if (tmin < 0) // ray is inside the cloud
	{
		intersectionPoint = rayPos + rayDir * tmax;
	}
	else  // ray outside the cloud, pointing torwards it
	{
		intersectionPoint = rayPos + rayDir * tmin;
	}

	return true;
}

bool tests::shadowVolumeTest(glm::uvec2 columnCoord, ShadowVolumeProperties& shadowVolumeProperties, const CloudProperties& cloudProperties)
{
	glm::vec4 accumulatedValue = glm::vec4(0.0f);
	glm::vec3 position = glm::vec3(0.0f);

	for (unsigned int z = 0; z < shadowVolumeProperties.voxelAxisCount; z++)
	{
		position = tests::calculateVoxelPosition(glm::uvec3(columnCoord, z), shadowVolumeProperties);
		if (tests::isInCloud(position, cloudProperties))
		{
			//accumulatedValue += sampleExtinction(position);
		}
	}

	return true;
}

bool tests::isInCloud(glm::vec3 point, const CloudProperties& cloudProperties)
{
	return !(point.x < cloudProperties.bounds[0].x ||
		point.y < cloudProperties.bounds[0].y ||
		point.z < cloudProperties.bounds[0].z ||
		point.x > cloudProperties.bounds[1].x ||
		point.y > cloudProperties.bounds[1].y ||
		point.z > cloudProperties.bounds[1].z);
}

glm::vec3 tests::calculateVoxelPosition(glm::uvec3 voxelIdx, ShadowVolumeProperties& shadowVolumeProperties)
{
	return (shadowVolumeProperties.bounds[0] +
		voxelIdx.x * shadowVolumeProperties.voxelSize * shadowVolumeProperties.right +
		voxelIdx.y * shadowVolumeProperties.voxelSize * shadowVolumeProperties.up +
		voxelIdx.z * shadowVolumeProperties.voxelSize * shadowVolumeProperties.lightDirection);
}

void tests::localSort()
{
	std::mt19937 gen(1); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution dist(0, 255);
	std::vector<unsigned int> codes;
	for (unsigned int i = 0; i < 1024; i++)
	{
		codes.push_back(dist(gen));
	}

	std::vector<unsigned int> temp(codes);
	std::vector<unsigned int> scatterOffsets(codes.size());
	for (unsigned int i = 0; i < 8; i++)
	{
		scatterOffsets.clear();
		radixSort(temp, scatterOffsets, i);
	}


	for (unsigned int i = 0; i < 1024; i++)
	{
		codes[scatterOffsets[i]] = temp[i];
	}
}

void tests::radixSort(std::vector<unsigned int>& keys, std::vector<unsigned int>& scatterOffsets, unsigned int nthShift)
{
	const size_t elementCount = keys.size();
	std::vector<unsigned int> temp(elementCount);
	std::vector<unsigned int> falses(elementCount);

	unsigned int startIdx, endIdx;
	unsigned int* offset = new unsigned int[4 * 1024];
	std::fill_n(offset, 4 * 1024, 1);

	memcpy(temp.data(), keys.data(), elementCount * sizeof(unsigned int));

	for (unsigned int thread = 0; thread < 256; thread++)
	{
		startIdx = thread * 4;
		endIdx = startIdx + 4;

		// Mark 1 and 0
		for (unsigned int i = startIdx; i < endIdx; i++)
		{
			falses[i] = ((temp[i] >> nthShift) & 1) ^ 1;
			scatterOffsets[i] = falses[i]; // reusing final buffer to save memory
		}
	}

	// Scan the 1s - Build sum in place up the tree
	unsigned int currentIdx;
	for (unsigned int d = static_cast<unsigned int>(elementCount) >> 1; d > 0; d >>= 1)
	{
		for (unsigned int thread = 0; thread < 256; thread++)
		{
			startIdx = thread * 4;
			endIdx = startIdx + 4;

			currentIdx = thread * 4;
			for (unsigned int i = startIdx; i < endIdx; i++)
			{
				if (i < d)
				{
					unsigned int ai = offset[currentIdx] * (2 * i + 1) - 1;
					unsigned int bi = offset[currentIdx] * (2 * i + 2) - 1;

					falses[bi] += falses[ai];
				}
				offset[currentIdx] *= 2;
				currentIdx++;
			}
		}
	}

	// Clear the last element 
	unsigned int totalFalses = falses[elementCount - 1];
	falses[elementCount - 1] = 0;

	// Traverse down tree & build scan
	for (unsigned int d = 1; d < elementCount; d *= 2)
	{

		for (unsigned int thread = 0; thread < 256; thread++)
		{
			startIdx = thread * 4;
			endIdx = startIdx + 4;

			currentIdx = thread * 4;
			for (unsigned int i = startIdx; i < endIdx; i++)
			{
				offset[currentIdx] >>= 1;
				if (i < d)
				{
					unsigned int ai = offset[currentIdx] * (2 * i + 1) - 1;
					unsigned int bi = offset[currentIdx] * (2 * i + 2) - 1;
					unsigned int t = falses[ai];
					falses[ai] = falses[bi];
					falses[bi] += t;
				}
				currentIdx++;
			}
		}
	}

	// Calculate scatter indexes
	for (unsigned int thread = 0; thread < 256; thread++)
	{
		startIdx = thread * 4;
		endIdx = startIdx + 4;
		for (unsigned int i = startIdx; i < endIdx; i++)
		{
			scatterOffsets[i] = (scatterOffsets[i] == 0) ? (i - falses[i] + totalFalses) : falses[i];
		}
	}

	delete[] offset;
}

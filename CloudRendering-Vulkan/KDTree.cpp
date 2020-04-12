#include "stdafx.h"
#include "KDTree.h"

#include "UniformBuffers.h"

KDTree::KDTree(Photon* photons, size_t count)
{
	m_photons = photons;
	m_count = count;
	m_axes.resize(m_count);

	Balance(0, m_count);
}


void KDTree::Balance(size_t begin, size_t end)
{
	// Array fully sorted
	if (end <= begin)
	{
		return;
	}

	// Find axis
	glm::vec3 cubeSize = CubeSize(begin, end);
	unsigned int axisIdx = GreatestAxis(cubeSize);

	// Swap photon position to build the heap
	size_t median = begin + (end - begin) / 2;
	//m_sortedPhotons[position] = m_photons[median];
	m_axes[begin];

	// Build subtrees
	Balance(begin, median);
	Balance(median + 1, end);
}

glm::vec3 KDTree::CubeSize(size_t begin, size_t end)
{
	glm::vec3 min(FLT_MAX);
	glm::vec3 max(FLT_MIN);

	for (size_t i = begin; i < end; i++)
	{
		const glm::vec3& pos = m_photons[i].position;
		for (unsigned int i = 0; i < 3; i++)
		{
			if (pos[i] < min[i])
			{
				min[i] = pos[i];
			}

			if (pos[i] > max[i])
			{
				max[i] = pos[i];
			}
		}
	}

	return max - min;
}

unsigned int KDTree::GreatestAxis(const glm::vec3& cubeSize)
{
	unsigned int axis = 0;
	for (unsigned int i = 1; i < 3; i++)
	{
		if (cubeSize[axis] < cubeSize[i])
		{
			axis = i;
		}
	}

	return axis;
}

const void KDTree::NearestNeighbors(const glm::vec3& point, unsigned int count, std::vector<glm::vec3> outPoints)
{

	return void();
}
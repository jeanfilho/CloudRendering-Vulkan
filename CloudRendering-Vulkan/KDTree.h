#pragma once

struct Photon;

class KDTree
{

public:
	KDTree(const KDTree&) = delete;
	KDTree& operator=(const KDTree&) = delete;

	KDTree(Photon* photons, size_t count);

	const void NearestNeighbors(const glm::vec3& point, unsigned int count, std::vector<glm::vec3> outPoints);

private:
	void Balance(size_t begin, size_t end);
	glm::vec3 CubeSize(size_t begin, size_t end);
	unsigned int GreatestAxis(const glm::vec3& cubeSize);

private:
	Photon* m_photons;
	size_t m_count;
	std::vector<unsigned int> m_axes;
};
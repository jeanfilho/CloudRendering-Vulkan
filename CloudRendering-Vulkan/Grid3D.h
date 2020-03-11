#pragma once

#include <stdexcept>

template<typename T>
class Grid3D
{
public:
	static Grid3D<T>* Load(const std::string& filename);

public:
	Grid3D(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ, double voxelSizeX = 1, double voxelSizeY = 1, double voxelSizeZ = 1);
	~Grid3D();

	T& operator()(size_t x, size_t y, size_t z);

	size_t GetByteSize();
	size_t GetElementSize();
	size_t GetSize();

	glm::dvec3 GetVoxelSize();
	glm::uvec3 GetVoxelCount();

	void* GetData();
	void Copy(void* src, size_t size);

	void Save(const std::string& filename);

private:
	std::vector<T> m_data;

	unsigned int m_countX;
	unsigned int m_countY;
	unsigned int m_countZ;

	double m_voxelSizeX;
	double m_voxelSizeY;
	double m_voxelSizeZ;
};


template<typename T>
inline Grid3D<T>* Grid3D<T>::Load(const std::string& filename)
{
	unsigned int sizeX = 0;
	unsigned int sizeY = 0;
	unsigned int sizeZ = 0;
	double voxelSizeX = 0;
	double voxelSizeY = 0;
	double voxelSizeZ = 0;

	std::ifstream in(filename, std::ofstream::in | std::ofstream::binary);
	in.read(reinterpret_cast<char*>(&sizeX), sizeof(unsigned int));
	in.read(reinterpret_cast<char*>(&sizeY), sizeof(unsigned int));
	in.read(reinterpret_cast<char*>(&sizeZ), sizeof(unsigned int));
	in.read(reinterpret_cast<char*>(&voxelSizeX), sizeof(double));
	in.read(reinterpret_cast<char*>(&voxelSizeY), sizeof(double));
	in.read(reinterpret_cast<char*>(&voxelSizeZ), sizeof(double));

	Grid3D<T>* grid = new Grid3D<T>(sizeX, sizeY, sizeZ, voxelSizeX, voxelSizeY, voxelSizeZ);

	in.read(reinterpret_cast<char*>(grid->m_data.data()), sizeof(T)* sizeX* sizeY* sizeZ);
	in.close();

	return grid;
}

template<typename T>
inline Grid3D<T>::Grid3D(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ, double voxelSizeX, double voxelSizeY, double voxelSizeZ)
{
	m_countX = sizeX;
	m_countY = sizeY;
	m_countZ = sizeZ;

	m_voxelSizeX = voxelSizeX;
	m_voxelSizeY = voxelSizeY;
	m_voxelSizeZ = voxelSizeZ;

	m_data.resize(static_cast<size_t>(m_countX)* static_cast<size_t>(m_countY)* static_cast<size_t>(m_countZ));
}

template<typename T>
inline Grid3D<T>::~Grid3D()
{
}

template<typename T>
inline T& Grid3D<T>::operator()(size_t x, size_t y, size_t z)
{
	if (x < m_countX && y < m_countY && z < m_countZ)
	{
		return m_data[x + y * m_countX + z * m_countX * m_countY];
	}

	throw std::out_of_range("Index out of 3D grid bounds");
	return T();
}

template<typename T>
inline size_t Grid3D<T>::GetByteSize()
{
	return m_data.size() * sizeof(T);
}

template<typename T>
inline size_t Grid3D<T>::GetSize()
{
	return m_data.size();
}

template<typename T>
inline glm::dvec3 Grid3D<T>::GetVoxelSize()
{
	return glm::dvec3(m_voxelSizeX, m_voxelSizeY, m_voxelSizeZ);
}

template<typename T>
inline glm::uvec3 Grid3D<T>::GetVoxelCount()
{
	return glm::uvec3(m_countX, m_countY, m_countZ);
}

template<typename T>
inline void* Grid3D<T>::GetData()
{
	return m_data.data();
}

template<typename T>
inline size_t Grid3D<T>::GetElementSize()
{
	return sizeof(T);
}

template<typename T>
inline void Grid3D<T>::Copy(void* src, size_t size)
{
	m_data.resize(size / sizeof(T));
	memcpy(m_data.data(), src, size);
}

template<typename T>
inline void Grid3D<T>::Save(const std::string& filename)
{
	std::ofstream out(filename, std::ofstream::out | std::ofstream::binary);
	out.write(reinterpret_cast<char*>(&m_countX), sizeof(unsigned int));
	out.write(reinterpret_cast<char*>(&m_countY), sizeof(unsigned int));
	out.write(reinterpret_cast<char*>(&m_countZ), sizeof(unsigned int));
	out.write(reinterpret_cast<char*>(&m_voxelSizeX), sizeof(double));
	out.write(reinterpret_cast<char*>(&m_voxelSizeY), sizeof(double));
	out.write(reinterpret_cast<char*>(&m_voxelSizeZ), sizeof(double));
	out.write(reinterpret_cast<char*>(m_data.data()), sizeof(T)* m_countX* m_countY* m_countZ);
	out.close();
}

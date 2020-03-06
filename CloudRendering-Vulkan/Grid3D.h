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

	void* GetData();
	void Copy(void* src, size_t size);

	void Save(const std::string& filename);

private:
	std::vector<T> m_data;
	size_t m_totalSize;

	unsigned int m_sizeX;
	unsigned int m_sizeY;
	unsigned int m_sizeZ;

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

	in.read(reinterpret_cast<char*>(grid->m_data.data()), sizeof(T) * sizeX * sizeY * sizeZ);
	in.close();

	return grid;
}

template<typename T>
inline Grid3D<T>::Grid3D(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ, double voxelSizeX, double voxelSizeY, double voxelSizeZ)
{
	m_sizeX = sizeX;
	m_sizeY = sizeY;
	m_sizeZ = sizeZ;

	m_voxelSizeX = voxelSizeX;
	m_voxelSizeY = voxelSizeY;
	m_voxelSizeZ = voxelSizeZ;

	m_totalSize = static_cast<size_t>(m_sizeX) * static_cast<size_t>(m_sizeY) * static_cast<size_t>(m_sizeZ);
	m_data.resize(m_totalSize);
}

template<typename T>
inline Grid3D<T>::~Grid3D()
{
}

template<typename T>
inline T& Grid3D<T>::operator()(size_t x, size_t y, size_t z)
{
	if (x < m_sizeX && y < m_sizeY && z < m_sizeZ)
	{
		return m_data[x + y * m_sizeX + z * m_sizeX * m_sizeY];
	}

	throw std::out_of_range("Index out of 3D grid bounds");
	return T();
}

template<typename T>
inline size_t Grid3D<T>::GetByteSize()
{
	return m_totalSize * sizeof(T);
}

template<typename T>
inline size_t Grid3D<T>::GetSize()
{
	return m_totalSize;
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
	memcpy(m_data.data(), src, size);
}

template<typename T>
inline void Grid3D<T>::Save(const std::string& filename)
{
	std::ofstream out(filename, std::ofstream::out | std::ofstream::binary);
	out.write(reinterpret_cast<char*>(&m_sizeX), sizeof(unsigned int));
	out.write(reinterpret_cast<char*>(&m_sizeY), sizeof(unsigned int));
	out.write(reinterpret_cast<char*>(&m_sizeZ), sizeof(unsigned int));
	out.write(reinterpret_cast<char*>(&m_voxelSizeX), sizeof(double));
	out.write(reinterpret_cast<char*>(&m_voxelSizeY), sizeof(double));
	out.write(reinterpret_cast<char*>(&m_voxelSizeZ), sizeof(double));
	out.write(reinterpret_cast<char*>(m_data.data()), sizeof(float)* m_sizeX* m_sizeY* m_sizeZ);
	out.close();
}

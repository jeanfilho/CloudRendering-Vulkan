//Dense Grid in ZYX order (z fastest, x slowest)
//Hence, loops iterates in the order x, y, z
#include <vector>
#include <fstream>

struct GridData
{
	unsigned int sizeX, sizeY, sizeZ;
	double voxelSizeX, voxelSizeY, voxelSizeZ;
	std::vector<float> data; //raw, linear data of size sizeX*sizeY*sizeZ

	size_t coordToOffset(size_t x, size_t y, size_t z) const { return z + sizeZ * (y + sizeY * x);} //for demonstration

	void save(const std::string& filename) //save in my .xyz format
	{
		std::ofstream out(filename, std::ofstream::out | std::ofstream::binary);
		out.write(reinterpret_cast<char*>(&sizeX), sizeof(unsigned int));
		out.write(reinterpret_cast<char*>(&sizeY), sizeof(unsigned int));
		out.write(reinterpret_cast<char*>(&sizeZ), sizeof(unsigned int));
		out.write(reinterpret_cast<char*>(&voxelSizeX), sizeof(double));
		out.write(reinterpret_cast<char*>(&voxelSizeY), sizeof(double));
		out.write(reinterpret_cast<char*>(&voxelSizeZ), sizeof(double));
		out.write(reinterpret_cast<char*>(data.data()), sizeof(float)*sizeX*sizeY*sizeZ);
		out.close();
	}
};
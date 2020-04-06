#include "stdafx.h"
#include "Utilities.h"

void utilities::GetOrthonormalBasis(const glm::vec3& fwd, glm::vec3& outRight, glm::vec3& outUp)
{
	float sz = fwd.z >= 0.0f ? 1.0f : -1.0f;
	float a = fwd.y / (1.0f + abs(fwd.z));
	float b = fwd.y * a;
	float c = -fwd.x * a;

	outRight = glm::vec3(fwd.z + sz * b, sz * c, -fwd.x);
	outUp = glm::vec3(c, 1.0f - b, -sz * fwd.y);
}



void utilities::ReadFile(const std::string& filename, std::vector<char>& outData)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	outData.resize(fileSize);

	file.seekg(0);
	file.read(outData.data(), fileSize);

	file.close();
}

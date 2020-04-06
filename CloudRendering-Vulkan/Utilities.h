#pragma once

namespace utilities
{
	void GetOrthonormalBasis(const glm::vec3& fwd, glm::vec3& outRight, glm::vec3& outUp);
	void ReadFile(const std::string& filename, std::vector<char>& outData);
}
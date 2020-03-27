#pragma once

struct CloudProperties;
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

	bool intersectCloud(CloudProperties cloudProperties, glm::vec3& rayDir, glm::vec3& rayPos, glm::vec3& intersectionPoint);
}

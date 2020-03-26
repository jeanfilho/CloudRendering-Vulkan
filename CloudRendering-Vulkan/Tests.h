#pragma once

struct CloudProperties;
namespace tests
{
	void RunTests();

	struct Ray
	{
		glm::vec3 pos;
		glm::vec3 dir;
	};

	int isNegativeSign(float value);

	bool intersectCloud(CloudProperties cloudProperties, glm::vec3& rayDir, glm::vec3& rayPos, glm::vec3& intersectionPoint);
}

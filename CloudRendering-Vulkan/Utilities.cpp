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

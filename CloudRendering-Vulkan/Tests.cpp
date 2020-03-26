#include "stdafx.h"
#include "Tests.h"

#include "UniformBuffers.h"
#include "Grid3D.h"

void tests::RunTests()
{

}

int tests::isNegativeSign(float value)
{
	return int(value < 0);
}

bool tests::intersectCloud(CloudProperties cloudProperties, glm::vec3& rayDir, glm::vec3& rayPos, glm::vec3& intersectionPoint)
{
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	glm::vec3 invdir = 1.0f / rayDir;
	int sign[3] = { isNegativeSign(invdir.x), isNegativeSign(invdir.y), isNegativeSign(invdir.z) };

	intersectionPoint = glm::vec3(FLT_MAX);

	tmin = (cloudProperties.bounds[sign[0]].x - rayPos.x) * invdir.x;
	tmax = (cloudProperties.bounds[1 - sign[0]].x - rayPos.x) * invdir.x;
	tymin = (cloudProperties.bounds[sign[1]].y - rayPos.y) * invdir.y;
	tymax = (cloudProperties.bounds[1 - sign[1]].y - rayPos.y) * invdir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	tzmin = (cloudProperties.bounds[sign[2]].z - rayPos.z) * invdir.z;
	tzmax = (cloudProperties.bounds[1 - sign[2]].z - rayPos.z) * invdir.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	// We just want intersections in front of the ray
	if (tmax < 0) // ray outside of the cloud, pointing away
	{
		return false;
	}
	else if (tmin < 0) // ray is inside the cloud
	{
		intersectionPoint = rayPos + rayDir * tmax;
	}
	else  // ray outside the cloud, pointing torwards it
	{
		intersectionPoint = rayPos + rayDir * tmin;
	}

	return true;
}

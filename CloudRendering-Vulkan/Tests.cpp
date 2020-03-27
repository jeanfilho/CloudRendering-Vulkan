#include "stdafx.h"
#include "Tests.h"

#include "UniformBuffers.h"
#include "Grid3D.h"

void tests::RunTests()
{
	glm::vec3 dir(0, 1, 0);
	glm::vec3 t0, t1;
	createOrthonormalBasis(dir, t0, t1);

	CloudProperties cloudProperties;
	cloudProperties.bounds[0] = glm::vec4(-100, -100, -100, 0);
	cloudProperties.bounds[1] = glm::vec4(100, 100, 100, 0);

	glm::vec3 intersectionPoint,
		rayPos = glm::vec3(0, 0, -100),
		rayDir = glm::normalize(glm::vec3(0, 0, 1));

	intersectCloud(cloudProperties, rayDir, rayPos, intersectionPoint);

	bool test = true;
}

void tests::createOrthonormalBasis(const glm::vec3& dir, glm::vec3& t0, glm::vec3& t1)
{
	float a = dir.y / (1.0f + dir.z);
	float b = dir.y * a;
	float c = -dir.x * a;

	t0 = glm::vec3(dir.z + b, c, -dir.x);
	t1 = glm::vec3(c, 1.0f - b, -dir.y);
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

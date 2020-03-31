#pragma once

#include <glm/glm.hpp>

struct CameraProperties
{
	glm::vec3 position = glm::vec3(0, 0, 0);
	int width = 1920;
	glm::vec3 forward = glm::vec3(0, 0, 1);
	int height = 1080;
	glm::vec3 right = glm::vec3(1, 0, 0);
	float nearPlane = 50.0f;
	glm::vec3 up = glm::vec3(0, 1, 0);
	float pixelSizeX = .1f;
	float pixelSizeY = .1f;

};

struct CloudProperties
{
	glm::vec4 bounds[2]{ glm::uvec4(0) ,glm::uvec4(0) };
	glm::uvec4 voxelCount = glm::uvec4(0);
	float maxExtinction = 0.2f;
};

struct Parameters
{
public:
	unsigned int maxRayBounces = 5;
private:
	float phaseG = 0.67f; // [-1, 1]
	float phaseOnePlusG2 = 1.0f + phaseG * phaseG;
	float phaseOneMinusG2 = 1.0f - phaseG * phaseG;
	float phaseOneOver2G = 0.5f / phaseG;
	bool isotropic = false;

public:
	void SetPhaseG(float value)
	{
		if (value > 1 || value < -1) return;

		phaseG = value;

		isotropic = (std::abs(phaseG) < 0.0001f);
		if (!isotropic)
		{
			phaseOnePlusG2 = 1.0f + phaseG * phaseG;
			phaseOneMinusG2 = 1.0f - phaseG * phaseG;
			phaseOneOver2G = 0.5f / phaseG;
		}
	}
};

struct ShadowVolumeProperties
{
	friend glm::vec3 tests::calculateVoxelPosition(glm::uvec3 voxelIdx, ShadowVolumeProperties& shadowVolumeProperties);
private:
	glm::vec4 bounds[2]{ glm::vec4(0), glm::vec4(0) };
	glm::vec4 lightDirection{ 0, 0, 1, 0 };
	glm::vec4 right{ 1, 0, 0, 0 };
	glm::vec4 up{ 0, 1, 0, 0 };

public:
	glm::uint voxelAxisCount = 200;
	glm::float32 voxelSize = 1.0f;

public:
	void SetLightDirection(glm::vec3 newDir)
	{
		glm::vec3 newRight, newUp;

		newDir = glm::normalize(newDir);
		if (newDir.y > .95f)
		{
			newDir = glm::vec3(0,1,0);
			newRight = glm::vec3(1,0,0);
			newUp = glm::vec3(0,0,-1);
		}
		else if(newDir.y < -.95f)
		{
			newDir = glm::vec3(0, -1, 0);
			newRight = glm::vec3(1, 0, 0);
			newUp = glm::vec3(0, 0, 1);
		}
		else
		{
			newRight = glm::normalize(glm::cross(newDir, glm::vec3(0, 1, 0)));
			newUp = glm::normalize(glm::cross(newRight, newDir));
		}

		lightDirection = glm::vec4(newDir.x, newDir.y, newDir.z, 0);
		right = glm::vec4(newRight.x, newRight.y, newRight.z, 0);
		up = glm::vec4(newUp.x, newUp.y, newUp.z, 0);

		UpdateOrigin(glm::distance(bounds[0], bounds[1]) / 2.f, (bounds[0] - bounds[1]) / 2.f);
	}

	void SetOrigin(glm::vec3 lowerBound, glm::vec3 upperBound)
	{
		float sphereRadius = glm::distance(lowerBound, upperBound) / 2.f;
		voxelSize = (2 * sphereRadius) / (voxelAxisCount - 1);

		UpdateOrigin(sphereRadius, (lowerBound + upperBound) / 2.f);
	}

private:
	void UpdateOrigin(float radius, glm::vec3 center)
	{
		glm::vec4 cornerPos = radius * (lightDirection - right - up);
		bounds[0] = cornerPos + glm::vec4(center, 0);
		bounds[1] = -cornerPos + glm::vec4(center, 0);
	}
};
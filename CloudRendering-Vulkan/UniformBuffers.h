#pragma once

#include <glm/glm.hpp>
#include "Tests.h"

struct PushConstants
{
	double  time = 0;
	int seed = 100;
	unsigned int frameCount = 0;
};

struct CameraProperties
{
	glm::vec3 position = glm::vec3(0, 0, -800);
private:
	int halfWidth = 800 / 2;
	glm::vec3 forward = glm::vec3(0, 0, 1);
	int halfHeight = 600 / 2;
	glm::vec3 right = glm::vec3(1, 0, 0);
	float nearPlane = 50.0f;
	glm::vec3 up = glm::vec3(0, 1, 0);
	float pixelSizeY = (2 * glm::tan(glm::radians(120 / 2.f)) * nearPlane) / (halfHeight * 2);
	float pixelSizeX = pixelSizeY * static_cast<float>(halfHeight) / static_cast<float>(halfWidth);

public:
	void SetRotation(glm::vec2& rotation)
	{
		rotation.x = std::clamp(rotation.x, -80.f, 80.f);

		forward = glm::rotate(glm::radians(rotation.x), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(rotation.y), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1, 0);
		utilities::GetOrthonormalBasis(forward, right, up);
	}

	int GetWidth() const
	{
		return halfWidth * 2;
	}
	int GetHeight() const
	{
		return halfHeight * 2;
	}

	void SetResolution(int newWidth, int newHeight)
	{
		halfWidth = newWidth / 2;
		halfHeight = newHeight / 2;
	}

	void SetFOV(float& fov)
	{
		fov = std::clamp(fov, 60.f, 150.f);
		float height = 2 * glm::tan(glm::radians(fov / 2.f)) * nearPlane;
		pixelSizeY = height / (halfHeight * 2);
		pixelSizeX = pixelSizeY * static_cast<float>(halfHeight) / static_cast<float>(halfWidth);
	}

	float GetNearPlane()
	{
		return nearPlane;
	}
};

struct CloudProperties
{
	glm::vec4 bounds[2]{ glm::uvec4(0) ,glm::uvec4(0) };
	glm::uvec4 voxelCount = glm::uvec4(0);
	float maxExtinction = 0.2f;
	float baseScaling = 1000.f;
	float densityScaling = 200.f;
};

struct Parameters
{
public:
	unsigned int maxRayBounces = 5;
	float lightIntensity = 5;
private:
	float phaseG = 0.00000001f; // [-1, 1]
	float phaseOnePlusG2 = 1.0f + phaseG * phaseG;
	float phaseOneMinusG2 = 1.0f - phaseG * phaseG;
	float phaseOneOver2G = 0.5f / phaseG;
	bool isotropic = std::abs(phaseG) < 0.0001f;

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

	float GetPhaseG()
	{
		return phaseG;
	}
};

struct ShadowVolumeProperties
{
	friend glm::vec3 tests::calculateVoxelPosition(glm::uvec3 voxelIdx, ShadowVolumeProperties& shadowVolumeProperties);

private:
	glm::vec4 bounds[2]{ glm::vec4(0), glm::vec4(0) };
	glm::vec4 lightDirection{ 1, -1, 0, 0 };
	glm::vec4 right{ 1, 0, 0, 0 };
	glm::vec4 up{ 0, 1, 0, 0 };
	glm::mat4 basisChange{ lightDirection, right, up, glm::vec4(0) };

public:
	glm::uint voxelAxisCount = 500;
	glm::float32 voxelSize = 1.0f;


public:
	void SetLightDirection(glm::vec3 newDir)
	{
		glm::vec3 newRight, newUp;
		newDir = glm::normalize(newDir);

		utilities::GetOrthonormalBasis(newDir, newRight, newUp);

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

	const glm::vec4& GetLightDirection()
	{
		return lightDirection;
	}

private:
	void UpdateOrigin(float radius, glm::vec3 center)
	{
		glm::vec4 cornerPos = radius * (-lightDirection - right - up);
		bounds[0] = glm::vec4(center, 1) + cornerPos;
		bounds[1] = glm::vec4(center, 1) - cornerPos;

		basisChange = glm::inverse(glm::mat4{ right, up, lightDirection, glm::vec4(0,0,0,1) });
	}
};

// Jarosz et al. - 2008 - Advanced Global Illumination using Photon Maps
struct Photon // 36 Bytes
{
	glm::vec4 position;
	glm::vec4 power;
	float phi;
	float theta;
	float __padding[2];
};

struct PhotonMapProperties
{
	glm::vec4 lightDirection = glm::normalize(glm::vec4( 1,-1, 0,0));

private:
	glm::vec4 bounds[2]{ {0,0,0,0}, {100,100,100,100} };
	glm::uvec3 cellCount{ 100, 100, 100 };
	float cellSize = (bounds[1] - bounds[0]).x / cellCount.x;
	const glm::uint photonSize = sizeof(Photon);
	float stepSize = 10;
	float sampleRadius = cellSize;
	float absorption = 0.0f;

public:
	void SetBounds(glm::vec4 bounds[2])
	{
		this->bounds[0] = bounds[0];
		glm::vec4 cloudSize = (bounds[1] - bounds[0]);

		int highestIdx = 0;
		float highestValue = FLT_MIN;
		for (int i = 0; i < 3; i++)
		{
			if (highestValue < glm::abs(cloudSize[i]))
			{
				highestIdx = i;
				highestValue = glm::abs(cloudSize[i]);
			}
		}

		this->bounds[1] = this->bounds[0] + glm::vec4(cloudSize[highestIdx]);
		cellSize = (this->bounds[1] - this->bounds[0]).x / cellCount.x;
		sampleRadius = cellSize;
	}

	uint32_t GetTotalSize() const
	{
		return cellCount.x * cellCount.y * cellCount.z;
	}
};
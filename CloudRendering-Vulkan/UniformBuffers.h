#pragma once

#include <glm/glm.hpp>

struct CameraProperties
{
	glm::vec3 position = glm::vec3(0, 0, -100);
	int width = 1920;
	glm::vec3 forward = glm::vec3(0, 0, 1);
	int height = 1080;
	glm::vec3 right = glm::vec3(1, 0, 0);
	float nearPlane = 50.0f;
	glm::vec3 up = glm::vec3(0, 1, 0);
	float pixelSizeX = 1;
	float pixelSizeY = 1;

};

struct CloudProperties
{
	glm::vec4 bounds[2]{ glm::uvec4(0) ,glm::uvec4(0) };
	glm::uvec4 voxelCount = glm::uvec4(0);
	float maxExtinction = 0.6f;

};

struct Parameters
{
public:
	unsigned int maxRayBounces = 5;
	float phaseG = 0.67f; // [-1, 1]
private:
	float phaseOnePlusG2 = 1.0f + phaseG * phaseG;
	float phaseOneMinusG2 = 1.0f - phaseG * phaseG;
	float phaseOneOver2G = 0.5f / phaseG;

public:
	void SetPhaseG(float value)
	{
		if (value > 1 || value < -1) return;

		phaseG = 0.67f;
		phaseOnePlusG2 = 1.0f + phaseG * phaseG;
		phaseOneMinusG2 = 1.0f - phaseG * phaseG;
		phaseOneOver2G = 0.5f / phaseG;
	}
};
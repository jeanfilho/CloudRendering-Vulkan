#pragma once

#include <stdint.h>

struct QueueFamilyIndices
{
	uint32_t graphicsFamily = UINT32_MAX;
	uint32_t computeFamily = UINT32_MAX;
	uint32_t presentFamily = UINT32_MAX;

	bool IsComplete()
	{
		return graphicsFamily != UINT32_MAX && computeFamily != UINT32_MAX && presentFamily != UINT32_MAX;
	}
};

#include "stdafx.h"
#include "Validation.h"

bool ValidCheck(const VkResult& result)
{
	if (result == VK_SUCCESS)
	{
		return true;
	}

	throw std::runtime_error("Failed to create Physical Device");
	return false;
}

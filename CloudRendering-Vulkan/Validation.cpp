#include "stdafx.h"
#include "Validation.h"

bool ValidCheck(const VkResult& result)
{
	if (result == VK_SUCCESS)
	{
		return true;
	}

	throw std::runtime_error("Vulkan error " + result);
	return false;
}

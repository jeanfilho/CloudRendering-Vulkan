#include "stdafx.h"
#include "Validation.h"

bool ValidCheck(const VkResult& result)
{
	if (result == VK_SUCCESS)
	{
		return true;
	}

	std::cout << "Vulkan Error " << result;
	return false;
}

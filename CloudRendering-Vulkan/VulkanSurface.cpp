#include "stdafx.h"
#include "VulkanSurface.h"

#include "VulkanInstance.h"

VulkanSurface::VulkanSurface(VulkanInstance* instance, GLFWwindow* window)
{
	m_instance = instance;

	VkWin32SurfaceCreateInfoKHR info = initializers::Win32SurfaceCreateInfo(window);
	ValidCheck(vkCreateWin32SurfaceKHR(m_instance->GetInstance(), &info, nullptr, &m_surface));
}

VulkanSurface::~VulkanSurface()
{
	if (m_surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(m_instance->GetInstance(), m_surface, nullptr);
	}
}

VkSurfaceKHR VulkanSurface::GetSurface()
{
	return m_surface;
}

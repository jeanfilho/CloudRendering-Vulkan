#pragma once

class VulkanInstance;

class VulkanSurface
{
public:
	VulkanSurface(VulkanInstance* instance, GLFWwindow* window);
	~VulkanSurface();

	VkSurfaceKHR GetSurface();

private:
	VulkanInstance* m_instance = nullptr;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
};
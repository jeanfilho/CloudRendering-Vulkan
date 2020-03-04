#pragma once

class VulkanDevice;

class VulkanSemaphore
{
public:
	VulkanSemaphore(VulkanDevice* device);
	~VulkanSemaphore();

	VkSemaphore GetSemaphore();

private:
	VulkanDevice* m_device = nullptr;
	VkSemaphore m_semaphore = VK_NULL_HANDLE;
};
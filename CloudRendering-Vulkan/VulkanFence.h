#pragma once

class VulkanDevice;

class VulkanFence
{
public:
	VulkanFence(VulkanDevice* device);
	~VulkanFence();

	VkFence& GetFence();

private:
	VulkanDevice* m_device = nullptr;
	VkFence m_fence = VK_NULL_HANDLE;
};
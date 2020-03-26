#pragma once

class VulkanDevice;

class VulkanSampler
{
public:
	VulkanSampler(VulkanDevice* device);
	~VulkanSampler();

	VkSampler GetSampler();

private:
	VulkanDevice* m_device = nullptr;
	VkSampler m_sampler = VK_NULL_HANDLE;
};
#pragma once

class VulkanDevice;

class VulkanImageView
{
public:
	VulkanImageView(VulkanDevice* device, VkImage image, VkFormat format);
	~VulkanImageView();

private:
	VulkanDevice* m_device = nullptr;
	VkImageView m_imageView = VK_NULL_HANDLE;
};
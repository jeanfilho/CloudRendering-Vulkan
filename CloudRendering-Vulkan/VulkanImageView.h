#pragma once

class VulkanDevice;
class VulkanImage;

class VulkanImageView
{
public:
	VulkanImageView(VulkanDevice* device, VulkanImage* image);
	VulkanImageView(VulkanDevice* device, VkImage image, VkFormat format, VkImageViewType viewType);
	VulkanImageView(VulkanImageView&& other) noexcept;
	VulkanImageView(const VulkanImageView&& other);
	~VulkanImageView();

	VkImageView GetImageView();

private:
	VulkanDevice* m_device = nullptr;
	VkImageView m_imageView = VK_NULL_HANDLE;
};
#pragma once

class VulkanSurface;
class VulkanDevice;

class VulkanSwapchain
{
private:
	static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

public:
	VulkanSwapchain(VulkanDevice* device, uint32_t width, uint32_t height);
	~VulkanSwapchain();

	VkSwapchainKHR GetSwapchain();
	std::vector<VkImage>& GetSwapchainImages();
	VkFormat GetImageFormat();
	VkExtent2D& GetExtent();

private:
	VulkanDevice* m_device = nullptr;

	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkExtent2D m_extent{};
	VkFormat m_format;

	std::vector<VkImage> m_swapchainImages;
};
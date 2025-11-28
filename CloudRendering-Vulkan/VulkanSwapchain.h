#pragma once

class VulkanSurface;
class VulkanDevice;

class VulkanSwapchain
{
private:
	VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

public:
	VulkanSwapchain(VulkanDevice* device, GLFWwindow* window);
	~VulkanSwapchain();

	VkSwapchainKHR GetSwapchain();
	std::vector<VkImage>& GetSwapchainImages();
	VkFormat GetImageFormat();
	VkExtent2D& GetExtent();
	unsigned int GetImageCount() const;

private:
	VulkanDevice* m_device = nullptr;

	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkExtent2D m_extent{};
	VkFormat m_format;

	std::vector<VkImage> m_swapchainImages;
};
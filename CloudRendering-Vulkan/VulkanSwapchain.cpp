#include "stdafx.h"
#include "VulkanSwapchain.h"

#include "VulkanDevice.h"
#include "SwapchainSupportDetails.h"

VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, GLFWwindow* window)
{
	m_device = device;

	SwapchainSupportDetails swapchainSupport = m_device->GetPhysicalDevice()->QuerySwapchainSupport(device->GetSurface());
	VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = ChoosePresentMode(swapchainSupport.presentModes);
	m_extent = ChooseExtent(swapchainSupport.capabilities, window);
	m_format = surfaceFormat.format;

	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
	{
		imageCount = swapchainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = initializers::SwapchainCreateInfo(
		m_device->GetSurface()->GetSurface(),
		m_device->GetPhysicalDevice()->GetQueueFamilyIndices(),
		m_device->GetPhysicalDevice()->QuerySwapchainSupport(m_device->GetSurface()),
		imageCount,
		surfaceFormat,
		presentMode,
		m_extent);

	ValidCheck(vkCreateSwapchainKHR(m_device->GetDevice(), &swapchainInfo, nullptr, &m_swapchain));

	// Get Swapchain images
	vkGetSwapchainImagesKHR(m_device->GetDevice(), m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device->GetDevice(), m_swapchain, &imageCount, m_swapchainImages.data());
}

VulkanSwapchain::~VulkanSwapchain()
{
	if (m_swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_device->GetDevice(), m_swapchain, nullptr);
	}
}

VkSwapchainKHR VulkanSwapchain::GetSwapchain()
{
	return m_swapchain;
}

std::vector<VkImage>& VulkanSwapchain::GetSwapchainImages()
{
	return m_swapchainImages;
}

VkFormat VulkanSwapchain::GetImageFormat()
{
	return m_format;
}

VkExtent2D& VulkanSwapchain::GetExtent()
{
	return m_extent;
}

VkSurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = { width, height };
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

#include "stdafx.h"
#include "VulkanPhysicalDevice.h"

#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "SwapchainSupportDetails.h"


VulkanPhysicalDevice* VulkanPhysicalDevice::CreatePhysicalDevice(VulkanInstance* instance, VulkanSurface* surface, const std::vector<const char*>& deviceExtensions)
{
	std::vector<VkPhysicalDevice> devices = GetAvailablePhysicalDevices(instance);
	VkPhysicalDevice secondaryDevice = VK_NULL_HANDLE;
	QueueFamilyIndices secondaryQueue;

	// Try to find a discrete GPU with present capabilities
	for (auto& device : devices)
	{
		QueueFamilyIndices queueFamily;
		if (CheckPhysicalDeviceSupported(device, queueFamily, surface, deviceExtensions))
		{
			VkPhysicalDeviceProperties physicalDeviceProperties{};
			vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				return new VulkanPhysicalDevice(instance, device, queueFamily, deviceExtensions);
			}
			else
			{
				VkPhysicalDevice secondaryDevice = device;
				QueueFamilyIndices secondaryQueue = queueFamily;
			}
		}
	}

	// If no D-GPU was found, return a lesser device if it was found
	if (secondaryDevice == VK_NULL_HANDLE)
	{
		return nullptr;
	}
	else
	{
		return new VulkanPhysicalDevice(instance, secondaryDevice, secondaryQueue, deviceExtensions);
	}
}

std::vector<VkPhysicalDevice> VulkanPhysicalDevice::GetAvailablePhysicalDevices(VulkanInstance* pInstance)
{
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(pInstance->GetInstance(), &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(pInstance->GetInstance(), &deviceCount, devices.data());

	return devices;
}

bool VulkanPhysicalDevice::CheckPhysicalDeviceSupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices, VulkanSurface* surface, const std::vector<const char*>& extensions)
{
	// Queue Family support check
	bool queueFamilySupport = CheckQueueFamilySupported(device, familyIndices, surface);

	// Extensions Check
	bool extensionsSupported = CheckDeviceExtensionsSupported(device, extensions);

	// SwapChain support
	bool swapchainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(device, surface);
		swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
	}

	return queueFamilySupport && extensionsSupported && swapchainAdequate;
}

bool VulkanPhysicalDevice::CheckQueueFamilySupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices, VulkanSurface* surface)
{
	uint32_t queueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	uint32_t i = 0;

	// Check if there is a family that has both graphics and compute capabilities
	for (auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface->GetSurface(), &presentSupport);
			if (presentSupport)
			{
				familyIndices.presentFamily = i;
			}
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				familyIndices.graphicsFamily = i;
			}
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				familyIndices.computeFamily = i;
			}
		}

		if (familyIndices.IsComplete())
		{
			return true;
		}

		i++;
	}

	return false;
}

bool VulkanPhysicalDevice::CheckDeviceExtensionsSupported(VkPhysicalDevice& device, const std::vector<const char*>& extensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanInstance* instance, VkPhysicalDevice& device, QueueFamilyIndices& indices, const std::vector<const char*>& deviceExtensions)
{
	m_instance = instance;
	m_device = device;
	m_familyIndices = indices;
	m_extensions = deviceExtensions;

	vkGetPhysicalDeviceProperties(device, &m_physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(device, &m_physicalDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(device, &m_physicalDeviceMemoryProperties);
}

VulkanPhysicalDevice::~VulkanPhysicalDevice()
{
}

VkPhysicalDevice& VulkanPhysicalDevice::GetPhysicaDevice()
{
	return m_device;
}

QueueFamilyIndices& VulkanPhysicalDevice::GetQueueFamilyIndices()
{
	return m_familyIndices;
}

VkPhysicalDeviceProperties& VulkanPhysicalDevice::GetPhysicalDeviceProperties()
{
	return m_physicalDeviceProperties;
}

VkPhysicalDeviceFeatures& VulkanPhysicalDevice::GetPhyisicalDeviceFeatures()
{
	return m_physicalDeviceFeatures;
}

VkPhysicalDeviceMemoryProperties& VulkanPhysicalDevice::GetPhysicalDeviceMemoryProperties()
{
	return m_physicalDeviceMemoryProperties;
}

std::vector<const char*>& VulkanPhysicalDevice::GetDeviceExtensions()
{
	return m_extensions;
}

SwapchainSupportDetails VulkanPhysicalDevice::QuerySwapchainSupport(VulkanSurface* surface)
{
	return QuerySwapchainSupport(m_device, surface);
}

SwapchainSupportDetails VulkanPhysicalDevice::QuerySwapchainSupport(VkPhysicalDevice& device, VulkanSurface* vulkanSurface)
{
	SwapchainSupportDetails details{};
	VkSurfaceKHR surface = vulkanSurface->GetSurface();

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

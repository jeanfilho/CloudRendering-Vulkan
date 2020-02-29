#include "stdafx.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanInstance.h"


VulkanPhysicalDevice* VulkanPhysicalDevice::CreatePhysicalDevice(VulkanInstance* pInstance)
{
	std::vector<VkPhysicalDevice> devices = GetAvailablePhysicalDevices(pInstance);
	VkPhysicalDevice secondaryDevice = VK_NULL_HANDLE;
	QueueFamilyIndices secondaryQueue;

	// Try to find a discrete GPU
	for (auto& device : devices)
	{
		QueueFamilyIndices queueFamily;
		if (CheckPhysicalDeviceSupported(device, queueFamily))
		{
			VkPhysicalDeviceProperties physicalDeviceProperties{};
			vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				return new VulkanPhysicalDevice(pInstance, device, queueFamily);
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
		return new VulkanPhysicalDevice(pInstance, secondaryDevice, secondaryQueue);
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

bool VulkanPhysicalDevice::CheckPhysicalDeviceSupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices)
{
	bool supportsQueueFamily = CheckQueueFamilySupported(device, familyIndices);
	return supportsQueueFamily;
}

bool VulkanPhysicalDevice::CheckQueueFamilySupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices)
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
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				familyIndices.graphicsIndices = i;
			}
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				familyIndices.computeIndices = i;
			}
		}

		if (familyIndices.graphicsIndices < UINT32_MAX && familyIndices.computeIndices < UINT32_MAX)
		{
			return true;
		}

		i++;
	}

	return false;
}

VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanInstance* instance, VkPhysicalDevice& device, QueueFamilyIndices& indices)
{
	m_instance = instance;
	m_device = device;
	m_familyIndices = indices;

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

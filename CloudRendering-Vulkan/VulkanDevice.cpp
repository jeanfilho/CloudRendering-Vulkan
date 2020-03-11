#include "stdafx.h"
#include "VulkanDevice.h"

VulkanDevice::VulkanDevice(VulkanInstance* instance, VulkanSurface* surface, VulkanPhysicalDevice* physicalDevice)
{
	m_instance = instance;
	m_physicalDevice = physicalDevice;
	m_surface = surface;

	QueueFamilyIndices& indices = m_physicalDevice->GetQueueFamilyIndices();
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily};

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float priority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		queueCreateInfos.push_back(initializers::DeviceQueueCreateInfo(queueFamily, priority));
	}

	VkDeviceCreateInfo createInfo = initializers::DeviceCreateInfo(queueCreateInfos, m_physicalDevice->GetPhyisicalDeviceFeatures(), m_physicalDevice->GetDeviceExtensions());
	ValidCheck(vkCreateDevice(m_physicalDevice->GetPhysicaDevice(), &createInfo, nullptr, &m_device));

	vkGetDeviceQueue(m_device, indices.computeFamily, 0, &m_computeQueue);
	vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
}

VulkanDevice::~VulkanDevice()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(m_device, nullptr);
	}
}

VulkanInstance* VulkanDevice::GetInstance()
{
	return m_instance;
}

VkDevice VulkanDevice::GetDevice()
{
	return m_device;
}

VulkanPhysicalDevice* VulkanDevice::GetPhysicalDevice()
{
	return m_physicalDevice;
}

VulkanSurface* VulkanDevice::GetSurface()
{
	return m_surface;
}

VkQueue VulkanDevice::GetComputeQueue()
{
	return m_computeQueue;
}

VkQueue VulkanDevice::GetGraphicsQueue()
{
	return m_graphicsQueue;
}

VkQueue VulkanDevice::GetPresentQueue()
{
	return m_presentQueue;
}

uint32_t VulkanDevice::FindMemoryType(VkMemoryPropertyFlags props, uint32_t typeFilter)
{
	for (uint32_t i = 0; i < m_physicalDevice->GetPhysicalDeviceMemoryProperties().memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (m_physicalDevice->GetPhysicalDeviceMemoryProperties().memoryTypes[i].propertyFlags & props) == props)
		{
			return i;
		}
	}

	assert(0 && "No available memory properties");
	return UINT32_MAX;
}

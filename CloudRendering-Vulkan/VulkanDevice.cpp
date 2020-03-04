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

	VkCommandPoolCreateInfo computePoolInfo = initializers::CommandPoolCreateInfo(m_physicalDevice->GetQueueFamilyIndices().computeFamily);
	ValidCheck(vkCreateCommandPool(m_device, &computePoolInfo, nullptr, &m_computeCommandPool));
}

VulkanDevice::~VulkanDevice()
{

	if (m_computeCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(m_device, m_computeCommandPool, nullptr);
	}

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

VkCommandPool& VulkanDevice::GetComputeCommandPool()
{
	return m_computeCommandPool;
}

void VulkanDevice::GetComputeCommand(VkCommandBuffer* buffers, uint32_t count)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = initializers::CommandBufferAllocateInfo(m_computeCommandPool, count);

	ValidCheck(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, buffers));
}

void VulkanDevice::FreeComputeCommand(VkCommandBuffer* buffers, uint32_t count)
{
	vkFreeCommandBuffers(m_device, m_computeCommandPool, count, buffers);
}

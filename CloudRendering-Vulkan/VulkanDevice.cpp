#include "stdafx.h"
#include "VulkanDevice.h"

#include "Initializers.h"
#include "VulkanPhysicalDevice.h"

VulkanDevice::VulkanDevice(VulkanInstance* instance, VulkanPhysicalDevice* physicalDevice)
{
	m_instance = instance;
	m_physicalDevice = physicalDevice;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float priority = 1.0f;
	queueCreateInfos.push_back(initializers::CreateDeviceQueueCreateInfo(m_physicalDevice->GetQueueFamilyIndices().graphicsIndices, priority));

	VkDeviceCreateInfo createInfo = initializers::CreateDeviceCreateInfo(queueCreateInfos, m_physicalDevice->GetPhyisicalDeviceFeatures());
	ValidCheck(vkCreateDevice(m_physicalDevice->GetPhysicaDevice(), &createInfo, nullptr, &m_device));

	vkGetDeviceQueue(m_device, m_physicalDevice->GetQueueFamilyIndices().computeIndices, 0, &m_computeQueue);

	VkCommandPoolCreateInfo computePoolInfo = initializers::CreateCommandPoolCreateInfo(m_physicalDevice->GetQueueFamilyIndices().computeIndices);
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

VkDevice* VulkanDevice::GetDevice()
{
	return &m_device;
}

VkQueue* VulkanDevice::GetComputeQueue()
{
	return &m_computeQueue;
}

VkCommandPool& VulkanDevice::GetComputeCommandPool()
{
	return m_computeCommandPool;
}

void VulkanDevice::GetComputeCommand(VkCommandBuffer* buffers, uint32_t count)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = initializers::CreateCommandBufferAllocateInfo(m_computeCommandPool, count);

	ValidCheck(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, buffers));
}

void VulkanDevice::FreeComputeCommand(VkCommandBuffer* buffers, uint32_t count)
{
	vkFreeCommandBuffers(m_device, m_computeCommandPool, count, buffers);
}

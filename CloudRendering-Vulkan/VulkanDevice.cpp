#include "stdafx.h"
#include "VulkanDevice.h"

#include "Initializers.h"
#include "VulkanPhysicalDevice.h"

VulkanDevice::VulkanDevice(VulkanInstance* pInstance, VulkanPhysicalDevice* pPhysicalDevice)
{
	m_pInstance = pInstance;
	m_pPhysicalDevice = pPhysicalDevice;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float priority = 1.0f;
	queueCreateInfos.push_back(initializers::CreateDeviceQueueCreateInfo(m_pPhysicalDevice->GetQueueFamilyIndices().graphicsIndices, priority));

	VkDeviceCreateInfo createInfo = initializers::CreateDeviceCreateInfo();
}

VulkanDevice::~VulkanDevice()
{
}

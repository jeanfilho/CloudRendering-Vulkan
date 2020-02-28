#pragma once

// Fwd. decl.
class VulkanPhysicalDevice;
class VulkanInstance;

class VulkanDevice
{
public:
	VulkanDevice(VulkanInstance* pInstance, VulkanPhysicalDevice* pPhysicalDevice);
	~VulkanDevice();

private:
	VulkanInstance* m_pInstance;
	VkDevice* m_pDevice;
	VulkanPhysicalDevice* m_pPhysicalDevice;
};
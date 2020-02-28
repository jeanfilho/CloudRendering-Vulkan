#pragma once
#include "QueueFamilyIndices.h"

// Fwd decl.
class VulkanInstance;

class VulkanPhysicalDevice
{
public:
	static VulkanPhysicalDevice* CreatePhysicalDevice(VulkanInstance* pInstance);

private:
	static std::vector<VkPhysicalDevice> GetAvailablePhysicalDevices(VulkanInstance* pInstance);
	static bool CheckPhysicalDeviceSupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices);
	static bool CheckQueueFamilySupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices);

public:
	~VulkanPhysicalDevice();

	VkPhysicalDevice& GetPhysicaDevice();
	const QueueFamilyIndices& GetQueueFamilyIndices();
	const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties();
	const VkPhysicalDeviceFeatures& GetPhyisicalDeviceFeatures();
	const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties();

private:
	VulkanPhysicalDevice(VulkanInstance* pInstance, VkPhysicalDevice& device, QueueFamilyIndices& indices);


	VulkanInstance* m_pInstance = nullptr;
	VkPhysicalDevice m_device{};
	QueueFamilyIndices m_familyIndices{};

	VkPhysicalDeviceProperties m_physicalDeviceProperties{};
	VkPhysicalDeviceFeatures m_physicalDeviceFeatures{};
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties{};
};
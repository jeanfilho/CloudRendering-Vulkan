#pragma once
#include "QueueFamilyIndices.h"

// Fwd decl.
class VulkanInstance;

class VulkanPhysicalDevice
{
public:
	static VulkanPhysicalDevice* CreatePhysicalDevice(VulkanInstance* instance);

private:
	static std::vector<VkPhysicalDevice> GetAvailablePhysicalDevices(VulkanInstance* instance);
	static bool CheckPhysicalDeviceSupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices);
	static bool CheckQueueFamilySupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices);

public:
	~VulkanPhysicalDevice();

	VkPhysicalDevice& GetPhysicaDevice();
	QueueFamilyIndices& GetQueueFamilyIndices();
	VkPhysicalDeviceProperties& GetPhysicalDeviceProperties();
	VkPhysicalDeviceFeatures& GetPhyisicalDeviceFeatures();
	VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties();

private:
	VulkanPhysicalDevice(VulkanInstance* instance, VkPhysicalDevice& device, QueueFamilyIndices& indices);


	VulkanInstance* m_instance = nullptr;
	VkPhysicalDevice m_device{};
	QueueFamilyIndices m_familyIndices{};

	VkPhysicalDeviceProperties m_physicalDeviceProperties{};
	VkPhysicalDeviceFeatures m_physicalDeviceFeatures{};
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties{};
};
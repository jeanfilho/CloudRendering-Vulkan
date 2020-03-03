#pragma once
#include "QueueFamilyIndices.h"
#include "SwapchainSupportDetails.h"

// Fwd decl.
class VulkanInstance;
class VulkanSurface;

class VulkanPhysicalDevice
{
public:
	static VulkanPhysicalDevice* CreatePhysicalDevice(VulkanInstance* pInstance, VulkanSurface* surface, const std::vector<const char*>& deviceExtensions);

private:
	static std::vector<VkPhysicalDevice> GetAvailablePhysicalDevices(VulkanInstance* instance);
	static bool CheckPhysicalDeviceSupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices, VulkanSurface* surface, const std::vector<const char*>& extensions);
	static bool CheckQueueFamilySupported(VkPhysicalDevice& device, QueueFamilyIndices& familyIndices, VulkanSurface* surface);
	static bool CheckDeviceExtensionsSupported(VkPhysicalDevice& device, const std::vector<const char*>& extensions);
	static SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice& device, VulkanSurface* surface);

public:
	~VulkanPhysicalDevice();

	VkPhysicalDevice& GetPhysicaDevice();
	QueueFamilyIndices& GetQueueFamilyIndices();
	VkPhysicalDeviceProperties& GetPhysicalDeviceProperties();
	VkPhysicalDeviceFeatures& GetPhyisicalDeviceFeatures();
	VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties();
	std::vector<const char*>& GetDeviceExtensions();
	SwapchainSupportDetails QuerySwapchainSupport(VulkanSurface* surface);


private:
	VulkanPhysicalDevice(VulkanInstance* instance, VkPhysicalDevice& device, QueueFamilyIndices& indices, const std::vector<const char*>& deviceExtensions);

	VulkanInstance* m_instance = nullptr;
	VkPhysicalDevice m_device{};
	QueueFamilyIndices m_familyIndices{};
	std::vector<const char*> m_extensions;

	VkPhysicalDeviceProperties m_physicalDeviceProperties{};
	VkPhysicalDeviceFeatures m_physicalDeviceFeatures{};
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties{};
};
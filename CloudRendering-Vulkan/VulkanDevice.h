#pragma once

#include "VulkanPhysicalDevice.h"
#include "VulkanSurface.h"

// Fwd. decl.
class VulkanInstance;

class VulkanDevice
{
public:
	VulkanDevice(VulkanInstance* instance, VulkanSurface* surface, VulkanPhysicalDevice* physicalDevice);
	~VulkanDevice();

	VulkanInstance* GetInstance();
	VulkanPhysicalDevice* GetPhysicalDevice();
	VulkanSurface* GetSurface();

	VkDevice GetDevice();
	VkQueue GetComputeQueue();
	VkCommandPool& GetComputeCommandPool();

	void GetComputeCommand(VkCommandBuffer* buffers, uint32_t count);
	void FreeComputeCommand(VkCommandBuffer* buffers, uint32_t count);

private:
	VulkanInstance* m_instance = nullptr;
	VulkanPhysicalDevice* m_physicalDevice = nullptr;
	VulkanSurface* m_surface = nullptr;

	VkDevice m_device;
	VkQueue m_computeQueue = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;
	VkCommandPool m_computeCommandPool;
};
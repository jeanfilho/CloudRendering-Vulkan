#pragma once

// Fwd. decl.
class VulkanPhysicalDevice;
class VulkanInstance;

class VulkanDevice
{
public:
	VulkanDevice(VulkanInstance* instance, VulkanPhysicalDevice* physicalDevice);
	~VulkanDevice();

	VulkanInstance* GetInstance();
	VkDevice* GetDevice();
	VkQueue* GetComputeQueue();
	VkCommandPool& GetComputeCommandPool();

	void GetComputeCommand(VkCommandBuffer* buffers, uint32_t count);
	void FreeComputeCommand(VkCommandBuffer* buffers, uint32_t count);

private:
	VulkanInstance* m_instance = nullptr;
	VkDevice m_device;
	VulkanPhysicalDevice* m_physicalDevice = nullptr;
	VkQueue m_computeQueue;
	VkCommandPool m_computeCommandPool;
};
#pragma once

class VulkanDevice;

class VulkanCommandPool
{
public:
	VulkanCommandPool(VulkanDevice* device, uint32_t queueFamilyIndex);
	~VulkanCommandPool();

	std::vector<VkCommandBuffer>& AllocateCommandBuffers(size_t size);
	void ClearCommandBuffers();

	VkCommandPool GetCommandPool();
	std::vector<VkCommandBuffer>& GetCommandBuffers();

private:
	VulkanDevice* m_device = nullptr;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_commandBuffers{};
};
#pragma once

class VulkanDevice;

class VulkanDescriptorPool
{
public:
	VulkanDescriptorPool(VulkanDevice* device, std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets = 10000);
	~VulkanDescriptorPool();

	void AllocateSets(std::vector<VkDescriptorSetLayout>& setLayouts, std::vector<VkDescriptorSet>& outSets);
	void Clear();

	std::vector<VkDescriptorSet>& GetDescriptorSets();
	VkDescriptorPool GetDescriptorPool();

private:
	VulkanDevice* m_device = nullptr;
	VkDescriptorPool m_pool = VK_NULL_HANDLE;

	std::vector<VkDescriptorSet> m_sets;
};
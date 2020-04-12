#pragma once

class VulkanDevice;
class RenderTechnique;

class VulkanDescriptorPool
{
public:
	VulkanDescriptorPool(VulkanDevice* device, std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets = 10000);
	~VulkanDescriptorPool();

	void AllocateSets(std::vector<VkDescriptorSetLayout>& setLayouts, std::vector<VkDescriptorSet>& outSets);
	void AllocateSets(RenderTechnique* renderTechnique, unsigned int setCount = 1);
	void Clear();

	std::vector<VkDescriptorSet>& GetDescriptorSets();
	VkDescriptorPool GetDescriptorPool();

private:
	VulkanDevice* m_device = nullptr;
	VkDescriptorPool m_pool = VK_NULL_HANDLE;

	std::vector<VkDescriptorSet> m_sets;
};
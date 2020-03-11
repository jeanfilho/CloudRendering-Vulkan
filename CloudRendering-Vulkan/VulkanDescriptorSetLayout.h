#pragma once

class VulkanDevice;

class VulkanDescriptorSetLayout
{
public:
	VulkanDescriptorSetLayout(VulkanDevice* device, std::vector<VkDescriptorSetLayoutBinding>& bindings);
	~VulkanDescriptorSetLayout();

	VkDescriptorSetLayout GetLayout();

private:
	VulkanDevice* m_device = nullptr;
	VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
};
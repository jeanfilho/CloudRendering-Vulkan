#include "stdafx.h"
#include "VulkanDescriptorSetLayout.h"

#include "VulkanDevice.h"

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice* device, std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	m_device = device;

	VkDescriptorSetLayoutCreateInfo layoutInfo = initializers::DescriptorSetLayoutCreateInfo(bindings);

	ValidCheck(vkCreateDescriptorSetLayout(m_device->GetDevice(), &layoutInfo, nullptr, &m_layout));
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	if (m_layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(m_device->GetDevice(), m_layout, nullptr);
	}
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::GetLayout()
{
	return m_layout;
}
 
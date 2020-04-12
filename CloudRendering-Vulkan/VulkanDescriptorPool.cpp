#include "stdafx.h"
#include "VulkanDescriptorPool.h"

#include "VulkanDevice.h"
#include "RenderTechnique.h"

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice* device, std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets /* = UINT32_MAX*/)
{
	m_device = device;

	VkDescriptorPoolCreateInfo poolInfo = initializers::DescriptorPoolCreateInfo(poolSizes, maxSets);
	ValidCheck(vkCreateDescriptorPool(m_device->GetDevice(), &poolInfo, nullptr, &m_pool));
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	if (m_pool != VK_NULL_HANDLE)
	{
		Clear();
		vkDestroyDescriptorPool(m_device->GetDevice(), m_pool, nullptr);
	}
}

void VulkanDescriptorPool::AllocateSets(RenderTechnique* renderTechnique, unsigned int setCount /* = 1*/)
{
	if (setCount == 0)
	{
		throw std::logic_error("[VulkanDescriptorPool::AllocateSets] Passed setCount = 0");
	}

	std::vector<VkDescriptorSetLayout> setLayouts;
	for (unsigned int i = 0; i < setCount; i++)
	{
		renderTechnique->GetDescriptorSetLayout(setLayouts);
	}
	
	auto& techniqueSets = renderTechnique->m_descriptorSets;
	techniqueSets.resize(setLayouts.size());

	VkDescriptorSetAllocateInfo setsInfo = initializers::DescriptorSetAllocateInfo(m_pool, setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));
	ValidCheck(vkAllocateDescriptorSets(m_device->GetDevice(), &setsInfo, techniqueSets.data()));

	for (auto& set : techniqueSets)
	{
		m_sets.push_back(set);
	}
}

void VulkanDescriptorPool::AllocateSets(std::vector<VkDescriptorSetLayout>& setLayouts, std::vector<VkDescriptorSet>& outSets)
{
	outSets.resize(setLayouts.size());

	VkDescriptorSetAllocateInfo setsInfo = initializers::DescriptorSetAllocateInfo(m_pool, setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));

	ValidCheck(vkAllocateDescriptorSets(m_device->GetDevice(), &setsInfo, outSets.data()));

	for (auto& set : outSets)
	{
		m_sets.push_back(set);
	}
}

void VulkanDescriptorPool::Clear()
{
	if (!m_sets.empty())
	{
		vkFreeDescriptorSets(m_device->GetDevice(), m_pool, static_cast<uint32_t>(m_sets.size()), m_sets.data());
	}
}

std::vector<VkDescriptorSet>& VulkanDescriptorPool::GetDescriptorSets()
{
	return m_sets;
}

VkDescriptorPool VulkanDescriptorPool::GetDescriptorPool()
{
	return m_pool;
}

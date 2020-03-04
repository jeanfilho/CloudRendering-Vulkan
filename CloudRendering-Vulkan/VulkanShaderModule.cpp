#include "stdafx.h"
#include "VulkanShaderModule.h"

#include "VulkanDevice.h"

VulkanShaderModule::VulkanShaderModule(VulkanDevice* device, std::vector<char>& code)
{
	m_device = device;

	VkShaderModuleCreateInfo createInfo = initializers::ShaderModuleCreateInfo(code);
	ValidCheck(vkCreateShaderModule(m_device->GetDevice(), &createInfo, nullptr, &m_shaderModule));
}

VulkanShaderModule::~VulkanShaderModule()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(m_device->GetDevice(), m_shaderModule, nullptr);
	}
}

VkShaderModule VulkanShaderModule::GetShaderModule()
{
	return m_shaderModule;
}

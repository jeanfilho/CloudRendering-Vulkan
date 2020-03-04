#pragma once

class VulkanDevice;

class VulkanShaderModule
{
public:
	VulkanShaderModule(VulkanDevice* device, std::vector<char>& code);
	~VulkanShaderModule();

	VkShaderModule GetShaderModule();

private:
	VulkanDevice* m_device = nullptr;
	VkShaderModule m_shaderModule = VK_NULL_HANDLE;
};
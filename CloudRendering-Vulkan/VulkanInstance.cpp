#include "stdafx.h"
#include "VulkanInstance.h"

VulkanInstance::VulkanInstance(const VulkanConfiguration& config)
{
	// Initialize GLFW required extensions
	uint32_t count;
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);;
	m_extensions.resize(count);
	for (uint32_t i = 0; i < count; i++)
	{
		m_extensions[i] = extensions[i];
	}

	//m_layers.push_back("VK_LAYER_LUNARG_api_dump");
	m_layers.push_back("VK_LAYER_KHRONOS_validation");
	m_extensions.push_back("VK_EXT_debug_report");
	
	VkApplicationInfo appInfo = initializers::ApplicationInfo(config);
	VkInstanceCreateInfo instanceInfo = initializers::InstanceCreateInfo(appInfo, m_layers, m_extensions);

	ValidCheck(vkCreateInstance(&instanceInfo, NULL, &m_instance));
}

VulkanInstance::~VulkanInstance()
{
	if (m_instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(m_instance, NULL);
	}
}

VkInstance& VulkanInstance::GetInstance()
{
	return m_instance;
}

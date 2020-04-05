#pragma once

class VulkanDevice;
class VulkanDescriptorPool;

class ImGUILayer
{
public:
	ImGUILayer(GLFWwindow* window, ImGui_ImplVulkan_InitInfo& initInfo, VkRenderPass renderPass);
	~ImGUILayer();

private:
	VulkanDevice* m_device = nullptr;
	VulkanDescriptorPool* m_descriptorPool = nullptr;
};
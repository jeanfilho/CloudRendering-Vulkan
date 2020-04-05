#pragma once

class VulkanDevice;
class VulkanDescriptorPool;
class VulkanImGUIRenderPass;
class VulkanSwapchain;

class ImGUILayer
{
public:
	ImGUILayer(GLFWwindow* window, VulkanDevice* device, VulkanSwapchain* swapchain, ImGui_ImplVulkan_InitInfo& initInfo);
	~ImGUILayer();

	VulkanImGUIRenderPass* GetRenderPass();

private:
	VulkanDevice* m_device = nullptr;
	VulkanDescriptorPool* m_descriptorPool = nullptr;
	VulkanImGUIRenderPass* m_renderPass = nullptr;
};
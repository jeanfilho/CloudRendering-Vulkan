#pragma once

#include "VulkanRenderPass.h"

class VulkanImGUIRenderPass : public VulkanRenderPass
{
public:
	VulkanImGUIRenderPass(VulkanDevice* device, VulkanSwapchain* swapchain);
	~VulkanImGUIRenderPass();

protected:
	virtual void AllocateResources(VulkanSwapchain* swapchain) override;
};
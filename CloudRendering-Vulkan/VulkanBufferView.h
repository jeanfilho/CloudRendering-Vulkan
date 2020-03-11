#pragma once

class VulkanDevice;
class VulkanBuffer;

class VulkanBufferView
{
public:
	VulkanBufferView(VulkanDevice* device, VulkanBuffer* buffer, VkFormat format);
	~VulkanBufferView();

	VkBufferView& GetBufferView();

private:
	VulkanDevice* m_device;
	VkBufferView m_view;
};

#pragma once

class VulkanDevice;

class VulkanBuffer
{
public:
	VulkanBuffer(VulkanDevice* device, void* data, size_t elementSize, VkBufferUsageFlagBits usageFlags, size_t count = 1);
	~VulkanBuffer();

	virtual void SetData();
	virtual void SetData(size_t count);
	virtual void SetData(size_t startIndex, size_t count);
	VkBuffer GetBuffer();
	VkDeviceSize GetSize();

private:
	void AllocateBuffer(VkBufferUsageFlagBits usageFlags);

private:
	VulkanDevice* m_device;
	VkBuffer m_buffer;
	
	void* m_ptr;
	size_t m_elementSize;
	size_t m_count;
	void* m_mappedMemory;

	VkDeviceSize m_totalSize;
	VkDeviceMemory m_deviceMemory;
};
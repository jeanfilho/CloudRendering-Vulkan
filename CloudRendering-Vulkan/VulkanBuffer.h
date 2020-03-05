#pragma once

class VulkanDevice;

class VulkanBuffer
{
public:
	VulkanBuffer(VulkanDevice* device, void* ptr, unsigned int elementSize, unsigned int count = 1);
	~VulkanBuffer();

	virtual void SetData();
	virtual void SetData(size_t count);
	virtual void SetData(size_t startIndex, size_t count);

private:
	void AllocateBuffer();
	uint32_t FindMemoryType(VkMemoryPropertyFlags props, uint32_t typeFilter);

private:
	VulkanDevice* m_device;
	void* m_ptr;
	unsigned int m_elementSize;
	unsigned int m_count;
	void* m_mappedMemory;

	VkDeviceSize m_totalSize;
	VkBuffer m_buffer;
	VkDeviceMemory m_deviceMemory;
};
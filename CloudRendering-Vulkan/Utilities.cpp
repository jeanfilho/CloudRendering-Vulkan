#include "stdafx.h"
#include "Utilities.h"

#include "VulkanDevice.h"
#include "VulkanCommandPool.h"

void utilities::GetOrthonormalBasis(const glm::vec3& fwd, glm::vec3& outRight, glm::vec3& outUp)
{
	float sz = fwd.z >= 0.0f ? 1.0f : -1.0f;
	float a = fwd.y / (1.0f + abs(fwd.z));
	float b = fwd.y * a;
	float c = -fwd.x * a;

	outRight = glm::vec3(fwd.z + sz * b, sz * c, -fwd.x);
	outUp = glm::vec3(c, 1.0f - b, -sz * fwd.y);
}



void utilities::ReadFile(const std::string& filename, std::vector<char>& outData)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	outData.resize(fileSize);

	file.seekg(0);
	file.read(outData.data(), fileSize);

	file.close();
}

VkCommandBuffer utilities::BeginSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* g_computeCommandPool)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = g_computeCommandPool->GetCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void utilities::EndSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool, VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(device->GetComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->GetComputeQueue());

	vkFreeCommandBuffers(device->GetDevice(), commandPool->GetCommandPool(), 1, &commandBuffer);
}

void utilities::CmdTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)

{
	VkImageMemoryBarrier imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.oldLayout = oldLayout;
	imgBarrier.newLayout = newLayout;
	imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.image = image;
	imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgBarrier.subresourceRange.baseMipLevel = 0;
	imgBarrier.subresourceRange.levelCount = 1;
	imgBarrier.subresourceRange.baseArrayLayer = 0;
	imgBarrier.subresourceRange.layerCount = 1;
	imgBarrier.srcAccessMask = 0;
	imgBarrier.dstAccessMask = 0;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier);
}

void utilities::CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, VkExtent3D imageExtent)

{
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = imageExtent;

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}


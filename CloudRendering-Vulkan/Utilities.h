#pragma once

class VulkanCommandPool;
class VulkanDevice;

namespace utilities
{
	void GetOrthonormalBasis(const glm::vec3& fwd, glm::vec3& outRight, glm::vec3& outUp);
	void ReadFile(const std::string& filename, std::vector<char>& outData);

	VkCommandBuffer BeginSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool);
	void EndSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool, VkCommandBuffer commandBuffer);

	void CmdTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, VkExtent3D imageExtent);
}
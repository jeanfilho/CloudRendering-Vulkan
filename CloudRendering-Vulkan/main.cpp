#include "stdafx.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanBufferView.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"
#include "VulkanPipelineLayout.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanImGUIRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanSemaphore.h"
#include "VulkanFence.h"
#include "VulkanComputePipeline.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDescriptorPool.h"


#include "Grid3D.h"
#include "Tests.h"
#include "UniformBuffers.h"
#include "ImGUILayer.h"


//--------------------------------------------------------------
// Shader Resources
//--------------------------------------------------------------
ShadowVolumeProperties g_shadowVolumeProperties;
VulkanBuffer* g_shadowVolumePropertiesBuffer;

CameraProperties g_cameraProperties;
VulkanBuffer* g_cameraPropertiesBuffer;

CloudProperties g_cloudProperties;
VulkanBuffer* g_cloudPropertiesBuffer;

Parameters g_parameters;
VulkanBuffer* g_parametersBuffer;

struct PushConstants
{
	double  time = 0;
	int seed = 100;
	unsigned int frameCount = 0;
} g_pushConstants;

//--------------------------------------------------------------
// Globals
//--------------------------------------------------------------
VulkanInstance* g_instance;
VulkanPhysicalDevice* g_physicalDevice;
VulkanDevice* g_device;
VulkanSurface* g_surface;
VulkanSwapchain* g_swapchain;
std::vector<VulkanImageView> g_swapchainImageViews;

VulkanCommandPool* g_graphicsCommandPool;
std::vector<VulkanFramebuffer*> g_framebuffers;

VulkanCommandPool* g_computeCommandPool;
VulkanDescriptorPool* g_computeDescriptorPool;

VulkanPipelineLayout* g_pathTracerPipelineLayout;
VulkanComputePipeline* g_pathTracerPipeline;
VulkanDescriptorSetLayout* g_pathTracerDescriptorSetLayout;		// compute bindings layout
std::vector<VkDescriptorSet> g_pathTracerDescriptorSets;		// compute bindings
VulkanShaderModule* g_pathTracerShader;
VulkanImage* g_pathTracerImage;
VulkanImageView* g_pathTraceImageView;

Grid3D<float>* g_cloudData;
VulkanImage* g_cloudImage;
VulkanImageView* g_cloudImageView;
VulkanSampler* g_cloudSampler;

VulkanPipelineLayout* g_shadowVolumePipelineLayout;
VulkanComputePipeline* g_shadowVolumePipeline;
VulkanShaderModule* g_shadowVolumeShader;
VulkanDescriptorSetLayout* g_shadowVolumeDescriptorSetLayout;
std::vector<VkDescriptorSet> g_shadowVolumeDescriptorSets;
VulkanImage* g_shadowVolumeImage;
VulkanImageView* g_shadowVolumeImageView;
VulkanSampler* g_shadowVolumeSampler;

ImGUILayer* g_imguiLayer = nullptr;

std::vector<VulkanSemaphore> g_imageAvailableSemaphores;
std::vector<VulkanSemaphore> g_renderFinishedSemaphores;
std::vector<VulkanFence> g_inFlightFences;
std::vector<VkFence> g_imagesInFlight;
uint32_t g_currentFrame = 0;

bool g_framebufferResized = false;

GLFWwindow* g_window;
const int MAX_FRAMES_IN_FLIGHT = 1;

double g_renderStartTime = 0;
double g_previousTime = 0;
unsigned int g_framesInSecond = 0;

//----------------------------------------------------------------------
// UI
//----------------------------------------------------------------------

char g_UICloudFile[1024];
std::string g_UICurrentCloudFile = " ";
float g_UIPhaseG = g_parameters.GetPhaseG();
float g_UISecondsPerFrame = 0;
glm::vec2 g_UICameraRotate{ 0, 0 };
glm::vec3 g_UILightDirection = g_shadowVolumeProperties.GetLightDirection();
int g_UICurrentResolution = 0;
int g_UIPreviousResolution = g_UICurrentResolution;
const char* RESOLUTIONS_NAMES[] = { "800x600", "1920x1080" };
const glm::ivec2 RESOLUTIONS[] = { {800, 600}, {1920, 1080} };
float g_UIFov = 90.f;

//----------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------
void UpdateTime()
{
	g_pushConstants.time = glfwGetTime();
	if (g_pushConstants.time - g_previousTime >= 1.0)
	{
		g_UISecondsPerFrame = static_cast<float>(1000.0 / double(g_framesInSecond));
		g_framesInSecond = 0;
		g_previousTime += 1.0;
	}
}

template<typename T>
void SetCloudProperties(Grid3D<T>* grid)
{
	glm::vec3 cloudSize{
		grid->GetVoxelSize().x * grid->GetVoxelCount().x * g_cloudProperties.baseScaling,
		grid->GetVoxelSize().y * grid->GetVoxelCount().y * g_cloudProperties.baseScaling,
		grid->GetVoxelSize().z * grid->GetVoxelCount().z * g_cloudProperties.baseScaling };

	g_cloudProperties.maxExtinction = grid->GetMajorant();
	g_cloudProperties.voxelCount = glm::uvec4(grid->GetVoxelCount(), 0);
	g_cloudProperties.bounds[0] = glm::vec4(
		-cloudSize.x / 2,
		-cloudSize.y / 2,
		0,
		0
	);
	g_cloudProperties.bounds[1] = -g_cloudProperties.bounds[0] + glm::vec4(0, 0, cloudSize.z, 0);
}


bool LoadCloudFile(const std::string filename)
{
	std::cout << "Loading cloud file...";

	if (g_cloudData)
	{
		delete g_cloudData;
		g_cloudData = nullptr;
	}

	g_cloudData = Grid3D<float>::Load("../models/" + filename);
	if (!g_cloudData)
	{
		std::cout << " ERROR: Failed to load file \"" + filename + "\" in models folder" << std::endl;
		return false;
	}

	g_UICurrentCloudFile = filename;

	SetCloudProperties(g_cloudData);

	std::cout << "OK" << std::endl;
	return true;
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	std::cout << "Button pressed";
}

VkCommandBuffer BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = g_computeCommandPool->GetCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(g_device->GetDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(g_device->GetComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(g_device->GetComputeQueue());

	vkFreeCommandBuffers(g_device->GetDevice(), g_computeCommandPool->GetCommandPool(), 1, &commandBuffer);
}

void CmdTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
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

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier);
}

void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, VkExtent3D imageExtent)
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

void RecordShadowVolumeUpdateCommands(VkCommandBuffer& commandBuffer)
{
	CmdTransitionImageLayout(commandBuffer, g_shadowVolumeImage->GetImage(), g_shadowVolumeImage->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, g_shadowVolumePipeline->GetPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, g_shadowVolumePipelineLayout->GetPipelineLayout(), 0, static_cast<uint32_t>(g_shadowVolumeDescriptorSets.size()), g_shadowVolumeDescriptorSets.data(), 0, nullptr);

	vkCmdDispatch(commandBuffer, g_shadowVolumeProperties.voxelAxisCount, g_shadowVolumeProperties.voxelAxisCount, 1);

	CmdTransitionImageLayout(commandBuffer, g_shadowVolumeImage->GetImage(), g_shadowVolumeImage->GetFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void RecordComputeCommands(uint32_t imageIndex)
{
	VkCommandBuffer& commandBuffer = g_computeCommandPool->GetCommandBuffers()[g_currentFrame];
	VkImage swapchainImage = g_swapchain->GetSwapchainImages()[imageIndex];
	VkImage pathTracerImage = g_pathTracerImage->GetImage();

	// Clear existing commands
	vkResetCommandBuffer(commandBuffer, 0);

	// Start recording commands
	vkBeginCommandBuffer(commandBuffer, &initializers::CommandBufferBeginInfo());

	// Change Result Image Layout to writeable
	CmdTransitionImageLayout(commandBuffer, pathTracerImage, g_pathTracerImage->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// Bind compute pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, g_pathTracerPipeline->GetPipeline());

	// Bind descriptor set (resources)
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, g_pathTracerPipelineLayout->GetPipelineLayout(), 0, static_cast<uint32_t>(g_pathTracerDescriptorSets.size()), g_pathTracerDescriptorSets.data(), 0, nullptr);

	// Push constants
	vkCmdPushConstants(commandBuffer, g_pathTracerPipelineLayout->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &g_pushConstants);

	// Start compute shader
	vkCmdDispatch(commandBuffer, g_cameraProperties.GetWidth(), g_cameraProperties.GetHeight(), 1);

	// Change swapchain image layout to dst blit
	CmdTransitionImageLayout(commandBuffer, swapchainImage, g_swapchain->GetImageFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Change result image layout to scr blit
	CmdTransitionImageLayout(commandBuffer, pathTracerImage, g_pathTracerImage->GetFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	// Copy result to swapchain image
	VkImageSubresourceLayers layers{};
	layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layers.layerCount = 1;
	layers.mipLevel = 0;

	VkExtent3D extents = g_pathTracerImage->GetExtent();
	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0,0,0 };
	blit.srcOffsets[1] = { static_cast<int32_t>(extents.width), static_cast<int32_t>(extents.height), static_cast<int32_t>(extents.depth) };
	blit.srcSubresource = layers;
	blit.dstOffsets[0] = { 0,0,0 };
	blit.dstOffsets[1] = { g_cameraProperties.GetWidth(), g_cameraProperties.GetHeight(), 1 };
	blit.dstSubresource = layers;

	vkCmdBlitImage(commandBuffer, pathTracerImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

	// Change swapchain image layout back to present
	CmdTransitionImageLayout(commandBuffer, swapchainImage, g_swapchain->GetImageFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	// End recording
	vkEndCommandBuffer(commandBuffer);
}

void RecordImGUICommands(uint32_t imageIndex)
{
	VkCommandBuffer commandBuffer = g_graphicsCommandPool->GetCommandBuffers()[g_currentFrame];
	VkImage swapchainImage = g_swapchain->GetSwapchainImages()[imageIndex];

	vkResetCommandBuffer(commandBuffer, 0);
	vkBeginCommandBuffer(commandBuffer, &initializers::CommandBufferBeginInfo());

	VkClearValue clearValue{};
	clearValue.color = VkClearColorValue();
	clearValue.depthStencil = VkClearDepthStencilValue();

	VkRenderPassBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.renderPass = g_imguiLayer->GetRenderPass()->GetRenderPass();
	info.framebuffer = g_framebuffers[imageIndex]->GetFramebuffer();
	info.renderArea.extent.width = g_cameraProperties.GetWidth();
	info.renderArea.extent.height = g_cameraProperties.GetHeight();
	info.clearValueCount = 1;
	info.pClearValues = &clearValue;
	vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	ValidCheck(vkEndCommandBuffer(commandBuffer));
}

void UpdateCloudData()
{
	vkDeviceWaitIdle(g_device->GetDevice());

	if (g_cloudImage)
	{
		delete g_cloudImage;
		delete g_cloudImageView;
		delete g_cloudSampler;
	}

	g_cloudImage = new VulkanImage(
		g_device,
		VK_FORMAT_R32_SFLOAT,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		static_cast<uint32_t>(g_cloudProperties.voxelCount.x),
		static_cast<uint32_t>(g_cloudProperties.voxelCount.y),
		static_cast<uint32_t>(g_cloudProperties.voxelCount.z));
	g_cloudImageView = new VulkanImageView(g_device, g_cloudImage);
	g_cloudSampler = new VulkanSampler(g_device);

	{
		VulkanBuffer stagingBuffer(g_device, g_cloudData->GetData(), g_cloudData->GetElementSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, g_cloudData->GetSize());
		stagingBuffer.SetData();

		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		CmdTransitionImageLayout(commandBuffer, g_cloudImage->GetImage(), g_cloudImage->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CmdCopyBufferToImage(commandBuffer, stagingBuffer.GetBuffer(), g_cloudImage->GetImage(), g_cloudImage->GetExtent());
		CmdTransitionImageLayout(commandBuffer, g_cloudImage->GetImage(), g_cloudImage->GetFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		EndSingleTimeCommands(commandBuffer);

		vkDeviceWaitIdle(g_device->GetDevice());
	}

	{
		g_cloudPropertiesBuffer->SetData();

		auto cloudBufferInfo = initializers::DescriptorBufferInfo(g_cloudPropertiesBuffer->GetBuffer(), 0, g_cloudPropertiesBuffer->GetSize());
		auto cloudImageInfo = initializers::DescriptorImageInfo(g_cloudSampler->GetSampler(), g_cloudImageView->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		std::vector<VkWriteDescriptorSet> writes;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			writes.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &cloudImageInfo));
			writes.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &cloudBufferInfo));
		};
		writes.push_back(initializers::WriteDescriptorSet(g_shadowVolumeDescriptorSets[0], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &cloudImageInfo));

		vkUpdateDescriptorSets(g_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}

	g_pushConstants.frameCount = 0;
	g_renderStartTime = g_pushConstants.time;
}

void UpdateShadowVolume()
{
	g_shadowVolumeProperties.SetLightDirection(g_UILightDirection);
	g_shadowVolumeProperties.SetOrigin(g_cloudProperties.bounds[0], g_cloudProperties.bounds[1]);
	g_shadowVolumePropertiesBuffer->SetData();

	// Update shadow volume descriptor set
	auto bufferInfo = initializers::DescriptorBufferInfo(g_shadowVolumePropertiesBuffer->GetBuffer(), 0, g_shadowVolumePropertiesBuffer->GetSize());
	VkWriteDescriptorSet write = initializers::WriteDescriptorSet(g_shadowVolumeDescriptorSets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo);
	vkUpdateDescriptorSets(g_device->GetDevice(), 1, &write, 0, nullptr);

	// Wait until any calculations are done
	vkQueueWaitIdle(g_device->GetComputeQueue());

	// Send commands for updating volume
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	RecordShadowVolumeUpdateCommands(commandBuffer);
	EndSingleTimeCommands(commandBuffer);

	// Update path tracer descriptor set
	std::vector<VkWriteDescriptorSet> writes;
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writes.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6, &bufferInfo));
	}
	vkUpdateDescriptorSets(g_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	g_pushConstants.frameCount = 0;
	g_renderStartTime = g_pushConstants.time;
}

void ClearSwapchain()
{
	vkDeviceWaitIdle(g_device->GetDevice());

	std::cout << "Clearing swapchain...";

	// Recreate result images. They should match the resolution of the screen
	delete g_pathTraceImageView;
	delete g_pathTracerImage;
	g_pathTraceImageView = nullptr;
	g_pathTracerImage = nullptr;

	// Recreate framebuffers
	for (auto& framebuffer : g_framebuffers)
	{
		delete framebuffer;
		framebuffer = nullptr;
	}

	// Recreate swapchain image views and swapchain
	g_swapchainImageViews.clear();
	delete g_swapchain;
	g_swapchain = nullptr;

	std::cout << "OK" << std::endl;
}

void CreateSwapchain()
{
	vkDeviceWaitIdle(g_device->GetDevice());

	int width = 0, height = 0;
	glfwGetFramebufferSize(g_window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(g_window, &width, &height);
		glfwWaitEvents();
	}
	if (g_swapchain)
	{
		vkDeviceWaitIdle(g_device->GetDevice());
		ClearSwapchain();
	}

	// Swapchain
	g_swapchain = new VulkanSwapchain(g_device, g_window);

	// Swapchain image views
	g_swapchainImageViews.reserve(g_swapchain->GetSwapchainImages().size());
	for (const VkImage& image : g_swapchain->GetSwapchainImages())
	{
		g_swapchainImageViews.emplace_back(g_device, image, g_swapchain->GetImageFormat(), VK_IMAGE_VIEW_TYPE_2D);
	}

	// Compute result image and view
	g_pathTracerImage = new VulkanImage(g_device, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, static_cast<uint32_t>(g_cameraProperties.GetWidth()), static_cast<uint32_t>(g_cameraProperties.GetHeight()));
	g_pathTraceImageView = new VulkanImageView(g_device, g_pathTracerImage);

	// Update compute bindings for output image
	std::vector<VkWriteDescriptorSet> writes;
	auto imageInfo = initializers::DescriptorImageInfo(VK_NULL_HANDLE, g_pathTraceImageView->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writes.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &imageInfo));
	};
	vkUpdateDescriptorSets(g_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	// Recreate command buffers
	g_computeCommandPool->AllocateCommandBuffers(g_swapchain->GetSwapchainImages().size());
	g_graphicsCommandPool->AllocateCommandBuffers(g_swapchain->GetSwapchainImages().size());


	// Create framebuffers for ImGUI if already present
	if (g_imguiLayer)
	{
		g_framebuffers.resize(g_swapchainImageViews.size());
		for (size_t i = 0; i < g_framebuffers.size(); i++)
		{
			g_framebuffers[i] = new VulkanFramebuffer(g_device, g_imguiLayer->GetRenderPass(), &g_swapchainImageViews[i], g_swapchain);
		}
	}
}

void Clear()
{

	delete g_imguiLayer;

	ClearSwapchain();

	std::cout << "Clearing allocations...";
	g_inFlightFences.clear();
	g_renderFinishedSemaphores.clear();
	g_imageAvailableSemaphores.clear();

	// Graphics
	delete g_graphicsCommandPool;

	// Shadow Volume
	delete g_shadowVolumePipeline;
	delete g_shadowVolumePipelineLayout;
	delete g_shadowVolumeShader;
	delete g_shadowVolumeDescriptorSetLayout;
	delete g_shadowVolumeImage;
	delete g_shadowVolumeImageView;
	delete g_shadowVolumeSampler;
	delete g_shadowVolumePropertiesBuffer;

	// Cloud Memory Allocations
	delete g_cloudImageView;
	delete g_cloudSampler;
	delete g_cloudImage;
	delete g_cameraPropertiesBuffer;
	delete g_cloudPropertiesBuffer;
	delete g_parametersBuffer;

	// Compute Resources
	delete g_pathTracerShader;
	delete g_computeCommandPool;
	delete g_computeDescriptorPool;
	delete g_pathTracerDescriptorSetLayout;
	delete g_pathTracerPipeline;
	delete g_pathTracerPipelineLayout;

	// Vulkan General Resources
	delete g_device;
	delete g_physicalDevice;
	delete g_surface;
	delete g_instance;

	glfwDestroyWindow(g_window);
	glfwTerminate();

	std::cout << "OK" << std::endl;
}

void DrawUI()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Rendering Stats");
	{
		ImGui::Text("FrameCount: %i", g_pushConstants.frameCount);
		ImGui::Text("Elapsed time: %.2f", g_pushConstants.time - g_renderStartTime);
		ImGui::Text("ms/frame: %.2f", g_UISecondsPerFrame);
	}
	ImGui::End();


	ImGui::Begin("Cloud File");
	{
		ImGui::Text("Current File: %s", g_UICurrentCloudFile.c_str());
		ImGui::InputText("", g_UICloudFile, 1024);
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			if (LoadCloudFile(std::string(g_UICloudFile)))
			{
				UpdateCloudData();
				UpdateShadowVolume();
			}
		}
	}
	ImGui::End();

	ImGui::Begin("Settings");
	{
		ImGui::Text("Parameters");
		ImGui::SliderFloat("Henyey-Greenstein G ", &g_UIPhaseG, -1, 1);

		ImGui::Separator();

		ImGui::Text("Camera");
		ImGui::InputFloat3("Position ", &g_cameraProperties.position[0], 2);
		ImGui::InputFloat2("Rotation ", &g_UICameraRotate[0], 2);
		ImGui::InputFloat("FOV", &g_UIFov);
		ImGui::Combo("Resolution", &g_UICurrentResolution, RESOLUTIONS_NAMES, IM_ARRAYSIZE(RESOLUTIONS_NAMES));

		ImGui::Separator();

		ImGui::Text("Lighting");
		ImGui::InputFloat3("Direction ", &g_UILightDirection[0], 2);
		ImGui::SliderFloat("Intensity ", &g_parameters.lightIntensity, 0, 10);

		ImGui::Separator();
		ImGui::Text("Cloud");
		ImGui::SliderFloat("Density", &g_cloudProperties.densityScaling, 0, 1000);

		if (ImGui::Button("Apply"))
		{

			if (g_UIPreviousResolution != g_UICurrentResolution)
			{
				g_UIPreviousResolution = g_UICurrentResolution;
				glfwSetWindowSize(g_window, RESOLUTIONS[g_UICurrentResolution].x, RESOLUTIONS[g_UICurrentResolution].y);

				ClearSwapchain();
				CreateSwapchain();
			}

			// Update data in memory

			g_parameters.SetPhaseG(g_UIPhaseG);
			g_cameraProperties.SetFOV(g_UIFov);
			g_cameraProperties.SetRotation(g_UICameraRotate);

			g_parametersBuffer->SetData();
			g_cameraPropertiesBuffer->SetData();
			g_cloudPropertiesBuffer->SetData();

			// Wait for all frames to finish
			vkDeviceWaitIdle(g_device->GetDevice());

			// Update sets in gpu
			auto parameterInfo = initializers::DescriptorBufferInfo(g_cameraPropertiesBuffer->GetBuffer(), 0, g_cameraPropertiesBuffer->GetSize());
			auto cameraPropertiesInfo = initializers::DescriptorBufferInfo(g_parametersBuffer->GetBuffer(), 0, g_parametersBuffer->GetSize());
			auto cloudBufferInfo = initializers::DescriptorBufferInfo(g_cloudPropertiesBuffer->GetBuffer(), 0, g_cloudPropertiesBuffer->GetSize());
			std::vector<VkWriteDescriptorSet> writes;
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				writes.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &parameterInfo));
				writes.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &cloudBufferInfo));
				writes.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &cameraPropertiesInfo));
			};
			vkUpdateDescriptorSets(g_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

			UpdateShadowVolume();
		}
	}
	ImGui::End();

	ImGui::Render();
}

void DrawFrame()
{
	// Wait for queue to finish if it is still running, and restore fence to original state
	vkWaitForFences(g_device->GetDevice(), 1, &g_inFlightFences[g_currentFrame].GetFence(), VK_TRUE, UINT64_MAX);

	// Acquire image from swapchain
	uint32_t imageIndex;
	ValidCheck(vkAcquireNextImageKHR(g_device->GetDevice(), g_swapchain->GetSwapchain(), UINT64_MAX, g_imageAvailableSemaphores[g_currentFrame].GetSemaphore(), VK_NULL_HANDLE, &imageIndex));

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (g_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(g_device->GetDevice(), 1, &g_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	// Mark the image as now being in use by this frame
	g_imagesInFlight[imageIndex] = g_inFlightFences[g_currentFrame].GetFence();

	// Record commands
	RecordComputeCommands(imageIndex);
	RecordImGUICommands(imageIndex);

	// Submit command buffer to queue
	std::vector<VkCommandBuffer> commandBuffers{ g_computeCommandPool->GetCommandBuffers()[g_currentFrame], g_graphicsCommandPool->GetCommandBuffers()[g_currentFrame] };
	VkSemaphore waitSemaphores[] = { g_imageAvailableSemaphores[g_currentFrame].GetSemaphore() };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
	VkSemaphore signalSemaphores[] = { g_renderFinishedSemaphores[g_currentFrame].GetSemaphore() };

	VkSubmitInfo submitInfo = initializers::SubmitInfo();
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	submitInfo.pCommandBuffers = commandBuffers.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Reset current fence
	vkResetFences(g_device->GetDevice(), 1, &g_inFlightFences[g_currentFrame].GetFence());

	// Submit commands to compute queue and signal the inFlightFence when finished
	ValidCheck(vkQueueSubmit(g_device->GetComputeQueue(), 1, &submitInfo, g_inFlightFences[g_currentFrame].GetFence()));

	// Present image
	VkSwapchainKHR swapchains[] = { g_swapchain->GetSwapchain() };
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(g_device->GetPresentQueue(), &presentInfo);

	// Advance to next frame
	g_currentFrame = (g_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RenderLoop()
{
	std::cout << "Render Loop started" << std::endl;
	while (!glfwWindowShouldClose(g_window))
	{
		glfwPollEvents();
		UpdateTime();
		g_pushConstants.seed = std::rand();

		if (!glfwGetWindowAttrib(g_window, GLFW_ICONIFIED))
		{
			DrawUI();
			DrawFrame();
			g_pushConstants.frameCount++;
			g_framesInSecond++;
		}
	}

	std::cout << "Render Loop stopped" << std::endl;
	vkDeviceWaitIdle(g_device->GetDevice());
	std::cout << "Device Finished" << std::endl;
}

void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	g_framebufferResized = true;
	g_cameraProperties.SetResolution(width, height);
}


void InitializeImGUI()
{
	ImGui_ImplVulkan_InitInfo initInfo{};

	initInfo.Instance = g_instance->GetInstance();
	initInfo.PhysicalDevice = g_physicalDevice->GetPhysicaDevice();
	initInfo.Device = g_device->GetDevice();
	initInfo.QueueFamily = g_physicalDevice->GetQueueFamilyIndices().graphicsFamily;
	initInfo.Queue = g_device->GetGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE; //Unused
	initInfo.DescriptorPool = VK_NULL_HANDLE; // Created by imguiLayer;
	initInfo.Allocator = nullptr; //Unused
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = static_cast<uint32_t>(g_swapchain->GetSwapchainImages().size());
	initInfo.CheckVkResultFn = ValidCheck;

	g_imguiLayer = new ImGUILayer(g_window, g_device, g_swapchain, initInfo);

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	EndSingleTimeCommands(commandBuffer);
}

void AllocateShaderResources()
{
	// Shader Modules
	std::vector<char> pathTracerSPV;
	std::vector<char> shadowVolumeSPV;

	utilities::ReadFile("../shaders/PathTracer.comp.spv", pathTracerSPV);
	g_pathTracerShader = new VulkanShaderModule(g_device, pathTracerSPV);
	utilities::ReadFile("../shaders/ShadowVolume.comp.spv", shadowVolumeSPV);
	g_shadowVolumeShader = new VulkanShaderModule(g_device, shadowVolumeSPV);

	// Buffers & Images
	g_cameraPropertiesBuffer = new VulkanBuffer(g_device, &g_cameraProperties, sizeof(CameraProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_cloudPropertiesBuffer = new VulkanBuffer(g_device, &g_cloudProperties, sizeof(CloudProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_parametersBuffer = new VulkanBuffer(g_device, &g_parameters, sizeof(Parameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	g_shadowVolumePropertiesBuffer = new VulkanBuffer(g_device, &g_shadowVolumeProperties, sizeof(ShadowVolumeProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_shadowVolumeImage = new VulkanImage(g_device, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, g_shadowVolumeProperties.voxelAxisCount, g_shadowVolumeProperties.voxelAxisCount, g_shadowVolumeProperties.voxelAxisCount);
	g_shadowVolumeImageView = new VulkanImageView(g_device, g_shadowVolumeImage);
	g_shadowVolumeSampler = new VulkanSampler(g_device);

	g_cameraPropertiesBuffer->SetData();
	g_parametersBuffer->SetData();

	// Transfer cloud data to device image and make it readable by the shader
	UpdateCloudData();

	// Update descriptor sets
	std::vector<VkWriteDescriptorSet> cloudSetWrites;
	auto parameterInfo = initializers::DescriptorBufferInfo(g_cameraPropertiesBuffer->GetBuffer(), 0, g_cameraPropertiesBuffer->GetSize());
	auto cameraPropertiesInfo = initializers::DescriptorBufferInfo(g_parametersBuffer->GetBuffer(), 0, g_parametersBuffer->GetSize());
	auto shadowImageInfo = initializers::DescriptorImageInfo(g_shadowVolumeSampler->GetSampler(), g_shadowVolumeImageView->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	auto cloudBufferInfo = initializers::DescriptorBufferInfo(g_cloudPropertiesBuffer->GetBuffer(), 0, g_cloudPropertiesBuffer->GetSize());
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		cloudSetWrites.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &parameterInfo));
		cloudSetWrites.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &cloudBufferInfo));
		cloudSetWrites.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &cameraPropertiesInfo));
		cloudSetWrites.push_back(initializers::WriteDescriptorSet(g_pathTracerDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &shadowImageInfo));
	};
	vkUpdateDescriptorSets(g_device->GetDevice(), static_cast<uint32_t>(cloudSetWrites.size()), cloudSetWrites.data(), 0, nullptr);

	// Update camera properties
	std::vector<VkWriteDescriptorSet> writes{
	};
	vkUpdateDescriptorSets(g_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	shadowImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	std::vector<VkWriteDescriptorSet> shadowVolumeWrites
	{
		initializers::WriteDescriptorSet(g_shadowVolumeDescriptorSets[0], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &shadowImageInfo),
		initializers::WriteDescriptorSet(g_shadowVolumeDescriptorSets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &cloudBufferInfo)
	};
	vkUpdateDescriptorSets(g_device->GetDevice(), static_cast<uint32_t>(shadowVolumeWrites.size()), shadowVolumeWrites.data(), 0, nullptr);
}

void AllocatePipelines()
{
	// Shadow volume pipeline;
	std::vector<VkPushConstantRange> svPushConstantRanges;
	std::vector<VkDescriptorSetLayout> svSetLayouts{ g_shadowVolumeDescriptorSetLayout->GetLayout() };
	g_shadowVolumePipelineLayout = new VulkanPipelineLayout(g_device, svSetLayouts, svPushConstantRanges);
	g_shadowVolumePipeline = new VulkanComputePipeline(g_device, g_shadowVolumePipelineLayout, g_shadowVolumeShader);

	// Path tracer pipeline;
	std::vector<VkPushConstantRange> ptPushConstantRanges
	{
		initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
	};
	std::vector<VkDescriptorSetLayout> ptSetLayouts;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		ptSetLayouts.push_back(g_pathTracerDescriptorSetLayout->GetLayout());
	};
	g_pathTracerPipelineLayout = new VulkanPipelineLayout(g_device, ptSetLayouts, ptPushConstantRanges);
	g_pathTracerPipeline = new VulkanComputePipeline(g_device, g_pathTracerPipelineLayout, g_pathTracerShader);

	// Create swapchain
	CreateSwapchain();
	if (!g_imguiLayer)
	{
		InitializeImGUI();
	}

	// Create framebuffers for ImGUI
	g_framebuffers.resize(g_swapchainImageViews.size());
	for (size_t i = 0; i < g_framebuffers.size(); i++)
	{
		g_framebuffers[i] = new VulkanFramebuffer(g_device, g_imguiLayer->GetRenderPass(), &g_swapchainImageViews[i], g_swapchain);
	}

	// Semaphores (GPU-GPU) and Fences (CPU-GPU)
	g_imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
	g_renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
	g_inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		g_imageAvailableSemaphores.emplace_back(g_device);
		g_renderFinishedSemaphores.emplace_back(g_device);
		g_inFlightFences.emplace_back(g_device);
	}
	g_imagesInFlight.resize(g_swapchain->GetSwapchainImages().size(), VK_NULL_HANDLE);
}

bool InitializeVulkan()
{
	std::cout << "Initializing Vulkan... ";

	VulkanConfiguration config{};
	config.applicationName = "Cloud Renderer";
	config.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	// Instance
	g_instance = new VulkanInstance(config);

	// Surface
	g_surface = new VulkanSurface(g_instance, g_window);

	// Physical Device
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	g_physicalDevice = VulkanPhysicalDevice::CreatePhysicalDevice(g_instance, g_surface, deviceExtensions);
	if (!g_physicalDevice)
	{
		throw std::runtime_error("Failed to create Physical Device");
		return false;
	}

	// Device
	g_device = new VulkanDevice(g_instance, g_surface, g_physicalDevice);

	// Command Pool
	g_computeCommandPool = new VulkanCommandPool(g_device, g_physicalDevice->GetQueueFamilyIndices().computeFamily);
	g_graphicsCommandPool = new VulkanCommandPool(g_device, g_physicalDevice->GetQueueFamilyIndices().graphicsFamily);

	// Path Tracer Descriptor Set Layout
	std::vector<VkDescriptorSetLayoutBinding> pathTracerSetLayoutBindings = {
		// Binding 0: Output 2D image (write)
		initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 1: Camera properties (read)
		initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 2: Cloud grid 3D sampler (read)
		initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		// Binding 3: Cloud Properties (read)
		initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 4: Parameters (read)
		initializers::DescriptorSetLayoutBinding(4, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 5: Shadow volume 3D sampler (read)
		initializers::DescriptorSetLayoutBinding(5, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		// Binding 5: Shadow volume properties (read)
		initializers::DescriptorSetLayoutBinding(6, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	};
	g_pathTracerDescriptorSetLayout = new VulkanDescriptorSetLayout(g_device, pathTracerSetLayoutBindings);

	// Shadow Volume Descriptor Set Layout
	std::vector<VkDescriptorSetLayoutBinding> shadowVolumeLayoutBindings = {
		// Binding 0: Output 3D image (write)
		initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 1: Shadow volume properties (read)
		initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 2: Cloud grid texel buffer (read)
		initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		// Binding 3: Cloud Properties (read)
		initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	};
	g_shadowVolumeDescriptorSetLayout = new VulkanDescriptorSetLayout(g_device, shadowVolumeLayoutBindings);


	// Compute Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 * MAX_FRAMES_IN_FLIGHT + 2),			// (Cloud Properties + Camera Properties + Parameters) * Frames in flight + Shadow Volume Properties
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 * MAX_FRAMES_IN_FLIGHT + 1),	// Cloud grid sampler3D, Shadow Volume sampler
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 * MAX_FRAMES_IN_FLIGHT + 1),			// Render image
	};
	std::vector<VkDescriptorSetLayout> pathTracerSetLayouts;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		pathTracerSetLayouts.push_back(g_pathTracerDescriptorSetLayout->GetLayout());
	};

	std::vector<VkDescriptorSetLayout> shadowVolumeSetLayouts;
	shadowVolumeSetLayouts.push_back(g_shadowVolumeDescriptorSetLayout->GetLayout());

	g_computeDescriptorPool = new VulkanDescriptorPool(g_device, poolSizes, static_cast<uint32_t>(shadowVolumeLayoutBindings.size() + pathTracerSetLayoutBindings.size() + MAX_FRAMES_IN_FLIGHT));

	g_computeDescriptorPool->AllocateSets(pathTracerSetLayouts, g_pathTracerDescriptorSets);
	g_computeDescriptorPool->AllocateSets(shadowVolumeSetLayouts, g_shadowVolumeDescriptorSets);

	AllocateShaderResources();
	AllocatePipelines();

	std::cout << "OK" << std::endl;

	return true;
}


bool InitializeGLFW()
{
	std::cout << "Initializing GLFW... ";

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	g_window = glfwCreateWindow(g_cameraProperties.GetWidth(), g_cameraProperties.GetHeight(), "Cloud Renderer", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(g_window, FramebufferResizeCallback);

	std::cout << "OK" << std::endl;
	return true;
}

int main()
{
	// Seed random
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

#ifdef _DEBUG
	tests::RunTests();
#endif

	// Create default data
	g_cloudData = new Grid3D<float>(100, 100, 100, .01, .01, .01);
	SetCloudProperties(g_cloudData);
	g_cameraProperties.SetFOV(g_UIFov);

	// Initialize Framework
	InitializeGLFW();
	InitializeVulkan();
	UpdateShadowVolume();

	RenderLoop();

	Clear();

	delete g_cloudData;

	return 0;
}
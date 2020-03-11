#include "stdafx.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanBufferView.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanShaderModule.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFrameBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanSemaphore.h"
#include "VulkanFence.h"
#include "VulkanComputePipeline.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDescriptorPool.h"

#include "Grid3D.h"

VulkanInstance* instance;
VulkanPhysicalDevice* physicalDevice;
VulkanDevice* device;
VulkanBuffer* buffer;
VulkanSurface* surface;
VulkanSwapchain* swapchain;
std::vector<VulkanImageView> swapchainImageViews;

VulkanComputePipeline* computePipeline;
VulkanPipelineLayout* computePipelineLayout;
VulkanCommandPool* computeCommandPool;
VulkanDescriptorSetLayout* computeDescriptorSetLayout;	// compute bindings layout
std::vector<VkDescriptorSet> computeDescriptorSets;		// compute bindings
VulkanDescriptorPool* computeDescriptorPool;
VulkanShaderModule* computeShader;
VulkanImage* computeResult;
VulkanImageView* computeResultView;

Grid3D<float>* cloudData;
VulkanBuffer* cloudBuffer;
VulkanBufferView* cloudBufferView;

std::vector<VulkanSemaphore> imageAvailableSemaphores;
std::vector<VulkanSemaphore> renderFinishedSemaphores;
std::vector<VulkanFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
uint32_t currentFrame = 0;

bool framebufferResized = false;

GLFWwindow* window;
const int MAX_FRAMES_IN_FLIGHT = 1;

struct CameraProperties
{
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 direction = glm::vec3(0, 0, -1);
	glm::vec2 size = glm::vec2(800, 600);
	int width = 1920;
	int height = 1080;

} cameraProperties;
VulkanBuffer* cameraPropertiesBuffer;

struct CloudProperties
{
	glm::vec3 origin;
	glm::dvec3 size;
	glm::uvec3 voxelCount;
	glm::dvec3 voxelSize;

} cloudProperties;
VulkanBuffer* cloudPropertiesBuffer;



std::vector<char> ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void RecordCommands(uint32_t imageIndex)
{
	VkCommandBuffer& commandBuffer = computeCommandPool->GetCommandBuffers()[currentFrame];
	VkImage swapchainImage = swapchain->GetSwapchainImages()[imageIndex];
	VkImage computeImage = computeResult->GetImage();

	// Clear existing commands
	vkResetCommandBuffer(commandBuffer, 0);

	// Start recording commands
	vkBeginCommandBuffer(commandBuffer, &initializers::CommandBufferBeginInfo());

	// Change Result Image Layout
	VkImageMemoryBarrier computeImgBarrier = {};
	computeImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	computeImgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	computeImgBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	computeImgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	computeImgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	computeImgBarrier.image = computeImage;
	computeImgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	computeImgBarrier.subresourceRange.baseMipLevel = 0;
	computeImgBarrier.subresourceRange.levelCount = 1;
	computeImgBarrier.subresourceRange.baseArrayLayer = 0;
	computeImgBarrier.subresourceRange.layerCount = 1;
	computeImgBarrier.srcAccessMask = 0;
	computeImgBarrier.dstAccessMask = 0;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &computeImgBarrier);

	// Change Swapchain Image Layout
	VkImageMemoryBarrier swapchainImgBarrier = {};
	swapchainImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	swapchainImgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	swapchainImgBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	swapchainImgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	swapchainImgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	swapchainImgBarrier.image = swapchainImage;
	swapchainImgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	swapchainImgBarrier.subresourceRange.baseMipLevel = 0;
	swapchainImgBarrier.subresourceRange.levelCount = 1;
	swapchainImgBarrier.subresourceRange.baseArrayLayer = 0;
	swapchainImgBarrier.subresourceRange.layerCount = 1;
	swapchainImgBarrier.srcAccessMask = 0;
	swapchainImgBarrier.dstAccessMask = 0;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &swapchainImgBarrier);

	// Bind compute pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->GetPipeline());

	// Bind descriptor set (resources)
	std::vector<VkDescriptorSet>& descriptorSets = computeDescriptorPool->GetDescriptorSets();
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout->GetPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

	// Start compute shader
	vkCmdDispatch(commandBuffer, cameraProperties.width, cameraProperties.height, 1);

	// Change swapchain image layout to dst blit
	swapchainImgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &swapchainImgBarrier);

	// Change result image layout to scr blit
	computeImgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &computeImgBarrier);

	// Copy result to swapchain image
	VkImageSubresourceLayers layers{};
	layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layers.layerCount = 1;
	layers.mipLevel = 0;

	VkExtent3D extents = computeResult->GetExtents();
	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0,0,0 };
	blit.srcOffsets[1] = { static_cast<int32_t>(extents.width), static_cast<int32_t>(extents.height), static_cast<int32_t>(extents.depth) };
	blit.srcSubresource = layers;
	blit.dstOffsets[0] = { 0,0,0 };
	blit.dstOffsets[1] = { cameraProperties.width, cameraProperties.height, 1 };
	blit.dstSubresource = layers;

	vkCmdBlitImage(commandBuffer, computeImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

	// Change swapchain image layout back to present
	swapchainImgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &swapchainImgBarrier);

	// End recording
	vkEndCommandBuffer(commandBuffer);
}

void ClearSwapchain()
{
	std::cout << "Clearing swapchain...";

	// Recreate result images. They should match the resolution of the screen
	delete computeResultView;
	delete computeResult;

	// Recreate the pipelines, since they depend on the swapchain
	delete computePipeline;
	delete computePipelineLayout;

	// Recreate swapchain image views and swapchain
	swapchainImageViews.clear();
	delete swapchain;

	std::cout << "OK" << std::endl;
}

void CreateSwapchain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	if (swapchain)
	{
		vkDeviceWaitIdle(device->GetDevice());
		ClearSwapchain();
	}

	// Swapchain
	swapchain = new VulkanSwapchain(device, window);

	// Swapchain image views
	swapchainImageViews.reserve(swapchain->GetSwapchainImages().size());
	for (const VkImage& image : swapchain->GetSwapchainImages())
	{
		swapchainImageViews.emplace_back(device, image, swapchain->GetImageFormat());
	}

	// Compute result image and view
	computeResult = new VulkanImage(device, static_cast<uint32_t>(cameraProperties.width), static_cast<uint32_t>(cameraProperties.height), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
	computeResultView = new VulkanImageView(device, computeResult->GetImage(), computeResult->GetFormat());

	// Update compute bindings for output image
	std::vector<VkWriteDescriptorSet> writes;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writes.push_back(initializers::WriteDescriptorSet(computeDescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &initializers::DescriptorImageInfo(VK_NULL_HANDLE, computeResultView->GetImageView(), VK_IMAGE_LAYOUT_GENERAL)));
	};
	vkUpdateDescriptorSets(device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	// Compute pipeline;
	std::vector<VkPushConstantRange> pushConstantRanges{};
	std::vector<VkDescriptorSetLayout> setLayouts;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		setLayouts.push_back(computeDescriptorSetLayout->GetLayout());
	};
	computePipelineLayout = new VulkanPipelineLayout(device, swapchain, setLayouts, pushConstantRanges);
	computePipeline = new VulkanComputePipeline(device, computePipelineLayout, computeShader);

	// Recreate command buffers
	if (!computeCommandPool)
	{
		computeCommandPool = new VulkanCommandPool(device, physicalDevice->GetQueueFamilyIndices().computeFamily);
	}
	computeCommandPool->AllocateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
}

bool IntializeVulkan()
{
	std::cout << "Initializing Vulkan... ";

	VulkanConfiguration config{};
	config.applicationName = "Cloud Renderer";
	config.applicationVersion = VK_MAKE_VERSION(0, 0, 0);

	// Instance
	instance = new VulkanInstance(config);

	// Surface
	surface = new VulkanSurface(instance, window);

	// Physical Device
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	physicalDevice = VulkanPhysicalDevice::CreatePhysicalDevice(instance, surface, deviceExtensions);
	if (!physicalDevice)
	{
		throw std::runtime_error("Failed to create Physical Device");
		return false;
	}

	// Device
	device = new VulkanDevice(instance, surface, physicalDevice);

	// Shader Modules
	auto computeSPV = ReadFile("../shaders/ComputeTest.spv");
	computeShader = new VulkanShaderModule(device, computeSPV);

	// Compute Descriptor Set Layout
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0: Output image (write)
		initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 1: Camera properties (read)
		initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 2: Cloud grid texel buffer (read)
		initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER),
		// Binding 3: Cloud Properties (read)
		initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
	};
	computeDescriptorSetLayout = new VulkanDescriptorSetLayout(device, setLayoutBindings);

	// Compute Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * MAX_FRAMES_IN_FLIGHT),			// Cloud Properties + Camera Properties
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 * MAX_FRAMES_IN_FLIGHT),	// Cloud grid sampler
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 * MAX_FRAMES_IN_FLIGHT),			// Render image
	};
	std::vector<VkDescriptorSetLayout> setLayouts;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		setLayouts.push_back(computeDescriptorSetLayout->GetLayout());
	};
	computeDescriptorPool = new VulkanDescriptorPool(device, poolSizes, static_cast<uint32_t>(setLayoutBindings.size() + MAX_FRAMES_IN_FLIGHT));
	computeDescriptorPool->AllocateSets(setLayouts, computeDescriptorSets);

	// Buffers
	cameraPropertiesBuffer = new VulkanBuffer(device, &cameraProperties, sizeof(CameraProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	cloudPropertiesBuffer = new VulkanBuffer(device, &cloudProperties, sizeof(CloudProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	cloudBuffer = new VulkanBuffer(device, cloudData->GetData(), static_cast<uint32_t>(cloudData->GetByteSize()), VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
	cloudBufferView = new VulkanBufferView(device, cloudBuffer, VK_FORMAT_R32_SFLOAT);

	// Update static descriptor sets
	std::vector<VkWriteDescriptorSet> writes;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writes.push_back(initializers::WriteDescriptorSet(computeDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2, &initializers::DescriptorBufferInfo(cloudBuffer->GetBuffer(), 0, cloudBuffer->GetSize()), &(cloudBufferView->GetBufferView()), 1));
		writes.push_back(initializers::WriteDescriptorSet(computeDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &initializers::DescriptorBufferInfo(cloudPropertiesBuffer->GetBuffer(), 0, cloudPropertiesBuffer->GetSize()), 1));
	};
	vkUpdateDescriptorSets(device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	// Swapchain and dependent objects
	CreateSwapchain();

	// Semaphores (GPU-GPU) and Fences (CPU-GPU)
	imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		imageAvailableSemaphores.emplace_back(device);
		renderFinishedSemaphores.emplace_back(device);
		inFlightFences.emplace_back(device);
	}
	imagesInFlight.resize(swapchain->GetSwapchainImages().size(), VK_NULL_HANDLE);

	std::cout << "OK" << std::endl;
	return true;
}


void Clear()
{
	std::cout << "Clearing allocations...";

	ClearSwapchain();

	inFlightFences.clear();
	renderFinishedSemaphores.clear();
	imageAvailableSemaphores.clear();

	// GPU Memory Allocations
	delete cloudBuffer;
	delete cameraPropertiesBuffer;
	delete cloudPropertiesBuffer;

	// Compute resources
	delete computeShader;
	delete computeCommandPool;
	delete computeDescriptorPool;
	delete computeDescriptorSetLayout;

	// Vulkan general resources
	delete device;
	delete physicalDevice;
	delete surface;
	delete instance;

	glfwDestroyWindow(window);
	glfwTerminate();

	std::cout << "OK" << std::endl;
}

void DrawFrame()
{
	// Wait for queue to finish if it is still running, and restore fence to original state
	vkWaitForFences(device->GetDevice(), 1, &inFlightFences[currentFrame].GetFence(), VK_TRUE, UINT64_MAX);

	// Acquire image from swapchain
	uint32_t imageIndex;
	vkAcquireNextImageKHR(device->GetDevice(), swapchain->GetSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame].GetSemaphore(), VK_NULL_HANDLE, &imageIndex);

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device->GetDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame].GetFence();

	// Update camera properties in GPU
	computeDescriptorPool->GetDescriptorSets();
	std::vector<VkWriteDescriptorSet> writes{
		initializers::WriteDescriptorSet(computeDescriptorSets[currentFrame], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &initializers::DescriptorBufferInfo(cameraPropertiesBuffer->GetBuffer(), 0, cameraPropertiesBuffer->GetSize()))
	};
	vkUpdateDescriptorSets(device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	// Record commands
	RecordCommands(imageIndex);

	// Submit command buffer to queue
	std::vector<VkCommandBuffer>& commandBuffers = computeCommandPool->GetCommandBuffers();
	VkSubmitInfo submitInfo = initializers::SubmitInfo();
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame].GetSemaphore() };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame].GetSemaphore() };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Reset current fence
	vkResetFences(device->GetDevice(), 1, &inFlightFences[currentFrame].GetFence());

	// Submit commands to compute queue and signal the inFlightFence when finished
	ValidCheck(vkQueueSubmit(device->GetComputeQueue(), 1, &submitInfo, inFlightFences[currentFrame].GetFence()));

	// Present image
	VkSwapchainKHR swapchains[] = { swapchain->GetSwapchain() };
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(device->GetPresentQueue(), &presentInfo);

	// Advance to next frame
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void MainLoop()
{
	std::cout << "Main Loop started" << std::endl;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		DrawFrame();
	}
	std::cout << "Main Loop stopped" << std::endl;
	vkDeviceWaitIdle(device->GetDevice());
	std::cout << "Device Finished" << std::endl;
}

void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	framebufferResized = true;
	cameraProperties.width = width;
	cameraProperties.height = height;
}

bool InitializeWindow()
{
	std::cout << "Initializing GLFW... ";

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(cameraProperties.width, cameraProperties.height, "Cloud Renderer", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

	std::cout << "OK" << std::endl;
	return true;
}


int main()
{
	// Load cloud from a file
	std::cout << "Loading cloud file...";
	cloudData = Grid3D<float>::Load("../models/cloud-1940.xyz");
	cloudProperties.origin = glm::vec3(0, 0, 0);
	cloudProperties.voxelSize = cloudData->GetVoxelSize();
	cloudProperties.voxelCount = cloudData->GetVoxelCount();
	cloudProperties.size = glm::dvec3(
		cloudProperties.voxelSize.x * cloudProperties.voxelCount.x,
		cloudProperties.voxelSize.y * cloudProperties.voxelCount.y,
		cloudProperties.voxelSize.z * cloudProperties.voxelCount.z
	);
	std::cout << "OK" << std::endl;

	// Initialize GLFW
	if (!InitializeWindow())
	{
		return 1;
	}

	// Initialize Vulkan
	if (!IntializeVulkan())
	{
		return 1;
	}

	MainLoop();
	Clear();

	delete cloudData;

	return 0;
}
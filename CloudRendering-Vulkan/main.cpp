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
#include "UniformBuffers.h"
#include "Tests.h"

//--------------------------------------------------------------
// Globals
//--------------------------------------------------------------
VulkanInstance* instance;
VulkanPhysicalDevice* physicalDevice;
VulkanDevice* device;
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
VulkanImage* cloudImage;
VulkanImageView* cloudImageView;
VulkanSampler* cloudSampler;

std::vector<VulkanSemaphore> imageAvailableSemaphores;
std::vector<VulkanSemaphore> renderFinishedSemaphores;
std::vector<VulkanFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
uint32_t currentFrame = 0;

bool framebufferResized = false;

GLFWwindow* window;
const int MAX_FRAMES_IN_FLIGHT = 1;

double previousTime = 0;
unsigned int frameCount = 0;

//--------------------------------------------------------------
// Shader Resources
//--------------------------------------------------------------
CameraProperties cameraProperties;
VulkanBuffer* cameraPropertiesBuffer;

CloudProperties cloudProperties;
VulkanBuffer* cloudPropertiesBuffer;

Parameters parameters;
VulkanBuffer* parametersBuffer;

struct PushConstants
{
	double  time = 0;
	int seed = 100;
	unsigned int frameCount = 0;
} pushConstants;

//----------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------
void UpdateTime()
{
	pushConstants.time = glfwGetTime();
	frameCount++;
	if (pushConstants.time - previousTime >= 1.0)
	{
		printf("%f ms/frame\n", 1000.0 / double(frameCount));
		frameCount = 0;
		previousTime += 1.0;
	}
}

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

VkCommandBuffer BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = computeCommandPool->GetCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &commandBuffer);

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

	vkQueueSubmit(device->GetComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->GetComputeQueue());

	vkFreeCommandBuffers(device->GetDevice(), computeCommandPool->GetCommandPool(), 1, &commandBuffer);
}

void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
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

void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, VkExtent3D imageExtent)
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

void RecordCommands(uint32_t imageIndex)
{
	VkCommandBuffer& commandBuffer = computeCommandPool->GetCommandBuffers()[currentFrame];
	VkImage swapchainImage = swapchain->GetSwapchainImages()[imageIndex];
	VkImage computeImage = computeResult->GetImage();

	// Clear existing commands
	vkResetCommandBuffer(commandBuffer, 0);

	// Start recording commands
	vkBeginCommandBuffer(commandBuffer, &initializers::CommandBufferBeginInfo());

	// Change Result Image Layout to writeable
	TransitionImageLayout(commandBuffer, computeImage, computeResult->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// Bind compute pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->GetPipeline());

	// Bind descriptor set (resources)
	std::vector<VkDescriptorSet>& descriptorSets = computeDescriptorPool->GetDescriptorSets();
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout->GetPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

	// Push constants
	vkCmdPushConstants(commandBuffer, computePipelineLayout->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pushConstants);

	// Start compute shader
	vkCmdDispatch(commandBuffer, cameraProperties.width, cameraProperties.height, 1);

	// Change swapchain image layout to dst blit
	TransitionImageLayout(commandBuffer, swapchainImage, computeResult->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Change result image layout to scr blit
	TransitionImageLayout(commandBuffer, computeImage, computeResult->GetFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	// Copy result to swapchain image
	VkImageSubresourceLayers layers{};
	layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layers.layerCount = 1;
	layers.mipLevel = 0;

	VkExtent3D extents = computeResult->GetExtent();
	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0,0,0 };
	blit.srcOffsets[1] = { static_cast<int32_t>(extents.width), static_cast<int32_t>(extents.height), static_cast<int32_t>(extents.depth) };
	blit.srcSubresource = layers;
	blit.dstOffsets[0] = { 0,0,0 };
	blit.dstOffsets[1] = { cameraProperties.width, cameraProperties.height, 1 };
	blit.dstSubresource = layers;

	vkCmdBlitImage(commandBuffer, computeImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

	// Change swapchain image layout back to present
	TransitionImageLayout(commandBuffer, swapchainImage, computeResult->GetFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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
		swapchainImageViews.emplace_back(device, image, swapchain->GetImageFormat(), VK_IMAGE_VIEW_TYPE_2D);
	}

	// Compute result image and view
	computeResult = new VulkanImage(device, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, static_cast<uint32_t>(cameraProperties.width), static_cast<uint32_t>(cameraProperties.height));
	computeResultView = new VulkanImageView(device, computeResult);

	// Update compute bindings for output image
	std::vector<VkWriteDescriptorSet> writes;
	auto imageInfo = initializers::DescriptorImageInfo(VK_NULL_HANDLE, computeResultView->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writes.push_back(initializers::WriteDescriptorSet(computeDescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &imageInfo));
	};
	vkUpdateDescriptorSets(device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	// Compute pipeline;
	std::vector<VkPushConstantRange> pushConstantRanges
	{
		initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
	};
	std::vector<VkDescriptorSetLayout> setLayouts;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		setLayouts.push_back(computeDescriptorSetLayout->GetLayout());
	};
	computePipelineLayout = new VulkanPipelineLayout(device, swapchain, setLayouts, pushConstantRanges);
	computePipeline = new VulkanComputePipeline(device, computePipelineLayout, computeShader);

	// Recreate command buffers
	computeCommandPool->AllocateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
}

bool IntializeVulkan()
{
	std::cout << "Initializing Vulkan... ";

	VulkanConfiguration config{};
	config.applicationName = "Cloud Renderer";
	config.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

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

	// Command Pool
	computeCommandPool = new VulkanCommandPool(device, physicalDevice->GetQueueFamilyIndices().computeFamily);

	// Shader Modules
	auto computeSPV = ReadFile("../shaders/PathTracer.comp.spv");
	computeShader = new VulkanShaderModule(device, computeSPV);

	// Compute Descriptor Set Layout
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0: Output image (write)
		initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 1: Camera properties (read)
		initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 2: Cloud grid texel buffer (read)
		initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		// Binding 3: Cloud Properties (read)
		initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 4: Parameters (read)
		initializers::DescriptorSetLayoutBinding(4, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
	};
	computeDescriptorSetLayout = new VulkanDescriptorSetLayout(device, setLayoutBindings);

	// Compute Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 * MAX_FRAMES_IN_FLIGHT),			// Cloud Properties + Camera Properties + Parameters
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 * MAX_FRAMES_IN_FLIGHT),	// Cloud grid sampler3D
		initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 * MAX_FRAMES_IN_FLIGHT),			// Render image
	};
	std::vector<VkDescriptorSetLayout> setLayouts;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		setLayouts.push_back(computeDescriptorSetLayout->GetLayout());
	};
	computeDescriptorPool = new VulkanDescriptorPool(device, poolSizes, static_cast<uint32_t>(setLayoutBindings.size() + MAX_FRAMES_IN_FLIGHT));
	computeDescriptorPool->AllocateSets(setLayouts, computeDescriptorSets);

	// Buffers & Images
	cameraPropertiesBuffer = new VulkanBuffer(device, &cameraProperties, sizeof(CameraProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	cloudPropertiesBuffer = new VulkanBuffer(device, &cloudProperties, sizeof(CloudProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	parametersBuffer = new VulkanBuffer(device, &parameters, sizeof(Parameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	cloudImage = new VulkanImage(
		device,
		VK_FORMAT_R32_SFLOAT,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		static_cast<uint32_t>(cloudProperties.voxelCount.x),
		static_cast<uint32_t>(cloudProperties.voxelCount.y),
		static_cast<uint32_t>(cloudProperties.voxelCount.z));
	cloudImageView = new VulkanImageView(device, cloudImage);
	cloudSampler = new VulkanSampler(device);

	// Transfer cloud data to device image and make it readable by the shader
	cloudPropertiesBuffer->SetData();

	VulkanBuffer stagingBuffer(device, cloudData->GetData(), cloudData->GetElementSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, cloudData->GetSize());
	stagingBuffer.SetData();

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	TransitionImageLayout(commandBuffer, cloudImage->GetImage(), cloudImage->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(commandBuffer, stagingBuffer.GetBuffer(), cloudImage->GetImage(), cloudImage->GetExtent());
	TransitionImageLayout(commandBuffer, cloudImage->GetImage(), cloudImage->GetFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	EndSingleTimeCommands(commandBuffer);

	vkDeviceWaitIdle(device->GetDevice());

	// Update static descriptor sets
	std::vector<VkWriteDescriptorSet> writes;
	auto imageInfo = initializers::DescriptorImageInfo(cloudSampler->GetSampler(), cloudImageView->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	auto bufferInfo = initializers::DescriptorBufferInfo(cloudPropertiesBuffer->GetBuffer(), 0, cloudPropertiesBuffer->GetSize());
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writes.push_back(initializers::WriteDescriptorSet(computeDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &imageInfo));
		writes.push_back(initializers::WriteDescriptorSet(computeDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &bufferInfo));
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
	ClearSwapchain();

	std::cout << "Clearing allocations...";
	inFlightFences.clear();
	renderFinishedSemaphores.clear();
	imageAvailableSemaphores.clear();


	// GPU Memory Allocations
	delete cloudImageView;
	delete cloudSampler;
	delete cloudImage;
	delete cameraPropertiesBuffer;
	delete cloudPropertiesBuffer;
	delete parametersBuffer;

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

	// Update camera buffer and parameters buffer
	cameraPropertiesBuffer->SetData();
	parametersBuffer->SetData();

	// Update camera properties and parameters in GPU
	computeDescriptorPool->GetDescriptorSets();
	auto parameterInfo = initializers::DescriptorBufferInfo(cameraPropertiesBuffer->GetBuffer(), 0, cameraPropertiesBuffer->GetSize());
	auto cameraPropertiesInfo = initializers::DescriptorBufferInfo(parametersBuffer->GetBuffer(), 0, parametersBuffer->GetSize());
	std::vector<VkWriteDescriptorSet> writes{
		initializers::WriteDescriptorSet(computeDescriptorSets[currentFrame], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &parameterInfo),
		initializers::WriteDescriptorSet(computeDescriptorSets[currentFrame], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &cameraPropertiesInfo)
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

		pushConstants.seed = std::rand();
		UpdateTime();
		DrawFrame();
		pushConstants.frameCount++;
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
#ifdef _DEBUG
	tests::RunTests();
#endif

	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// Load cloud from a file
	std::cout << "Loading cloud file...";

	cloudData = Grid3D<float>::Load("../models/cloud-1090.xyz");
	//cloudData = new Grid3D<float>(2, 2, 2, 200, 200, 200);
	//std::vector<float> testData =
	//{ 1, 0, 0, 0,
	//	0, 0, 0, 0 };
	//cloudData->Copy(testData.data(), testData.size() * sizeof(float));

	glm::vec3 cloudSize{
		cloudData->GetVoxelSize().x * 1000,
		cloudData->GetVoxelSize().y * 1000,
		cloudData->GetVoxelSize().z * 1000 };

	cloudProperties.maxExtinction = cloudData->GetMajorant();
	cloudProperties.voxelCount = glm::uvec4(cloudData->GetVoxelCount(), 0);
	cloudProperties.bounds[0] = glm::vec4(
		-cloudSize.x / 2 * cloudProperties.voxelCount.x,
		-cloudSize.y / 2 * cloudProperties.voxelCount.y,
		0,
		0
		);
	cloudProperties.bounds[1] = -cloudProperties.bounds[0] + glm::vec4(0, 0, cloudSize.z / 2 * cloudProperties.voxelCount.z, 0);
	std::cout << "OK" << std::endl;

	parameters.maxRayBounces = 3;
	parameters.SetPhaseG(0);

	cameraProperties.position = glm::vec3(0, 0, -100);


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
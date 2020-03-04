#include "stdafx.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "VulkanImageView.h"
#include "VulkanShaderModule.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFrameBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanSemaphore.h"
#include "VulkanFence.h"

VulkanInstance* instance;
VulkanPhysicalDevice* physicalDevice;
VulkanDevice* device;
VulkanBuffer* buffer;
VulkanSurface* surface;
VulkanSwapchain* swapchain;
std::vector<VulkanImageView> swapchainImageViews;
VulkanPipelineLayout* pipelineLayout;
VulkanShaderModule* vertShaderModule;
VulkanShaderModule* fragShaderModule;
VulkanRenderPass* renderPass;
VulkanGraphicsPipeline* graphicsPipeline;
std::vector<VulkanFramebuffer> swapchainFramebuffers;
VulkanCommandPool* commandPool;

std::vector<VulkanSemaphore> imageAvailableSemaphores;
std::vector<VulkanSemaphore> renderFinishedSemaphores;
std::vector<VulkanFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
size_t currentFrame = 0;

GLFWwindow* window;
const int WIDTH = 800;
const int HEIGHT = 800;
const int MAX_FRAMES_IN_FLIGHT = 2;

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

void DrawFrame()
{
	vkWaitForFences(device->GetDevice(), 1, &inFlightFences[currentFrame].GetFence(), VK_TRUE, UINT64_MAX);
	vkResetFences(device->GetDevice(), 1, &inFlightFences[currentFrame].GetFence());

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device->GetDevice(), swapchain->GetSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame].GetSemaphore(), VK_NULL_HANDLE, &imageIndex);

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device->GetDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame].GetFence();

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame].GetSemaphore() };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame].GetSemaphore() };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandPool->GetCommandBuffers()[imageIndex];

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device->GetDevice(), 1, &inFlightFences[currentFrame].GetFence());

	ValidCheck(vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame].GetFence()));

	VkSwapchainKHR swapChains[] = { swapchain ->GetSwapchain() };
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(device->GetPresentQueue(), &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RecordCommands()
{
	auto& commandBuffers = commandPool->GetCommandBuffers();
	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		// Start recording
		ValidCheck(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass->GetRenderPass();
		renderPassInfo.framebuffer = swapchainFramebuffers[i].GetFramebuffer();

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchain->GetExtent();

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->GetPipeline());
		vkCmdDraw(commandBuffers[i], 0, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		//Finish recording
		ValidCheck(vkEndCommandBuffer(commandBuffers[i]));
	}
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

bool InitializeWindow()
{
	std::cout << "Initializing GLFW... ";

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Cloud Renderer", nullptr, nullptr);

	std::cout << "OK" << std::endl;
	return true;
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
	
	// Swapchain
	swapchain = new VulkanSwapchain(device, WIDTH, HEIGHT);

	// Swapchain image views
	swapchainImageViews.reserve(swapchain->GetSwapchainImages().size());
	for (const VkImage& image : swapchain->GetSwapchainImages())
	{
		swapchainImageViews.emplace_back(device, image, swapchain->GetImageFormat());
	}

	// Shader Modules
	auto vert = ReadFile("../shaders/VertexShader.spv");
	auto frag = ReadFile("../shaders/FragmentShader.spv");
	vertShaderModule = new VulkanShaderModule(device, vert);
	fragShaderModule = new VulkanShaderModule(device, frag);
	std::vector<VulkanShaderModule*> shaderModules{ vertShaderModule, fragShaderModule };

	// Pipeline Layout
	pipelineLayout = new VulkanPipelineLayout(device, swapchain);

	// Render pass
	renderPass = new VulkanRenderPass(device, swapchain);

	// Graphics pipeline;
	graphicsPipeline = new VulkanGraphicsPipeline(device, pipelineLayout, renderPass, shaderModules);

	// Framebuffers
	swapchainFramebuffers.reserve(swapchainImageViews.size());
	for (VulkanImageView& swapchainImageView : swapchainImageViews)
	{
		swapchainFramebuffers.emplace_back(device, renderPass, &swapchainImageView, swapchain);
	}

	// Command pools and buffers
	commandPool = new VulkanCommandPool(device, physicalDevice->GetQueueFamilyIndices().graphicsFamily);
	commandPool->AllocateCommandBuffers(swapchainFramebuffers.size());

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
	
	inFlightFences.clear();
	renderFinishedSemaphores.clear();
	imageAvailableSemaphores.clear();
	delete commandPool;
	swapchainFramebuffers.clear();
	delete graphicsPipeline;
	delete renderPass;
	delete pipelineLayout;
	delete fragShaderModule;
	delete vertShaderModule;
	swapchainImageViews.clear();
	delete swapchain;
	delete device;
	delete physicalDevice;
	delete surface;
	delete instance;

	glfwDestroyWindow(window);
	glfwTerminate();

	std::cout << "OK" << std::endl;
}

int main()
{
	if (!InitializeWindow())
	{
		return 1;
	}

	if (!IntializeVulkan())
	{
		return 1;
	}
	RecordCommands();
	MainLoop();
	Clear();

	return 0;
}
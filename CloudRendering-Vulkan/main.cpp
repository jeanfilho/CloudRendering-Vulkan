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

bool framebufferResized = false;

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
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		// Finish recording
		ValidCheck(vkEndCommandBuffer(commandBuffers[i]));
	}
}

void ClearSwapchain()
{
	std::cout << "Clearing swapchain...";

	swapchainFramebuffers.clear();

	commandPool->ClearCommandBuffers();

	delete graphicsPipeline;
	delete pipelineLayout;
	delete renderPass;

	swapchainImageViews.clear();

	delete swapchain;
	std::cout << "OK" << std::endl;
}

void Clear()
{
	std::cout << "Clearing allocations...";

	ClearSwapchain();

	inFlightFences.clear();
	renderFinishedSemaphores.clear();
	imageAvailableSemaphores.clear();
	delete commandPool;
	delete fragShaderModule;
	delete vertShaderModule;
	delete device;
	delete physicalDevice;
	delete surface;
	delete instance;

	glfwDestroyWindow(window);
	glfwTerminate();

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

	// Pipeline Layout
	pipelineLayout = new VulkanPipelineLayout(device, swapchain);

	// Render pass
	renderPass = new VulkanRenderPass(device, swapchain);

	// Graphics pipeline;
	std::vector<VulkanShaderModule*> shaderModules{ vertShaderModule, fragShaderModule };
	graphicsPipeline = new VulkanGraphicsPipeline(device, pipelineLayout, renderPass, shaderModules);

	// Framebuffers
	swapchainFramebuffers.reserve(swapchainImageViews.size());
	for (VulkanImageView& swapchainImageView : swapchainImageViews)
	{
		swapchainFramebuffers.emplace_back(device, renderPass, &swapchainImageView, swapchain);
	}

	// Recreate command buffers
	if (!commandPool)
	{
		commandPool = new VulkanCommandPool(device, physicalDevice->GetQueueFamilyIndices().graphicsFamily);
	}
	commandPool->AllocateCommandBuffers(swapchainFramebuffers.size());
	RecordCommands();
}

void DrawFrame()
{
	vkWaitForFences(device->GetDevice(), 1, &inFlightFences[currentFrame].GetFence(), VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;

	// Try to take an image from the swapchain and check if the swapchain is up to date
	VkResult result = vkAcquireNextImageKHR(device->GetDevice(), swapchain->GetSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame].GetSemaphore(), VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		CreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

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

	VkSwapchainKHR swapChains[] = { swapchain->GetSwapchain() };
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	// Present the result and check again if the swapchain needs to be recreated
	result = vkQueuePresentKHR(device->GetPresentQueue(), &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		CreateSwapchain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

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
}

bool InitializeWindow()
{
	std::cout << "Initializing GLFW... ";

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Cloud Renderer", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

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

	// Shader Modules
	auto vert = ReadFile("../shaders/VertexShader.spv");
	auto frag = ReadFile("../shaders/FragmentShader.spv");
	vertShaderModule = new VulkanShaderModule(device, vert);
	fragShaderModule = new VulkanShaderModule(device, frag);

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
	MainLoop();
	Clear();

	return 0;
}
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

GLFWwindow* window;
const int WIDTH = 800;
const int HEIGHT = 800;

std::vector<char> readFile(const std::string& filename)
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

void MainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
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
	auto vert = readFile("../shaders/VertexShader.spv");
	auto frag = readFile("../shaders/FragmentShader.spv");
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

	std::cout << "OK" << std::endl;
	return true;
}

void Clear()
{
	std::cout << "Clearing allocations...";
	
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

	VkCommandBuffer* commands = new VkCommandBuffer[3];
	device->GetComputeCommand(commands, 3);

	float* arr = new float[3];
	for (int i = 0; i < 3; i++)
	{
		arr[i] = (float)i;
	}

	buffer = new VulkanBuffer(device, arr, sizeof(float), 3);
	buffer->SetData();

	device->FreeComputeCommand(commands, 3);


	MainLoop();

	delete[] arr;
	delete buffer;

	Clear();

	return 0;
}
#include "stdafx.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "VulkanImageView.h"

VulkanInstance* instance;
VulkanPhysicalDevice* physicalDevice;
VulkanDevice* device;
VulkanBuffer* buffer;
VulkanSurface* surface;
VulkanSwapchain* swapchain;
std::vector<VulkanImageView> swapchainImageViews;

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

	instance = new VulkanInstance(config);
	surface = new VulkanSurface(instance, window);

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	physicalDevice = VulkanPhysicalDevice::CreatePhysicalDevice(instance, surface, deviceExtensions);
	if (!physicalDevice)
	{
		throw std::runtime_error("Failed to create Physical Device");
		return false;
	}

	device = new VulkanDevice(instance, surface, physicalDevice);
	swapchain = new VulkanSwapchain(device, WIDTH, HEIGHT);

	swapchainImageViews.reserve(swapchain->GetSwapchainImages().size());
	for (const VkImage& image : swapchain->GetSwapchainImages())
	{
		swapchainImageViews.emplace_back(device, image, swapchain->GetImageFormat());
	}


	std::cout << "OK" << std::endl;
	return true;
}

void Clear()
{
	std::cout << "Clearing allocations...";

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
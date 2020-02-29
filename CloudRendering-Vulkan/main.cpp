#include "stdafx.h"

#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanDevice.h"

VulkanInstance* instance;
VulkanPhysicalDevice* physicalDevice;
VulkanDevice* device;

bool InitializeVulkan()
{
	std::cout << "Initializing Vulkan..." << std::endl;

	VulkanConfiguration config{};
	config.applicationName = "Cloud Renderer";
	config.applicationVersion = VK_MAKE_VERSION(0, 0, 0);

	instance = new VulkanInstance(config);
	if (!instance)
	{
		std::cout << "Failed to create Vulkan instance" << std::endl;
		return false;
	}

	physicalDevice = VulkanPhysicalDevice::CreatePhysicalDevice(instance);
	if (!physicalDevice)
	{
		std::cout << "Failed to create Vulkan Physical Device" << std::endl;
		return false;
	}

	device = new VulkanDevice(instance, physicalDevice);
	if (!device)
	{
		std::cout << "Failed to create Vulkan Device" << std::endl;
		return false;
	}

	return true;
}

void Clear()
{
	std::cout << "Clearing memory..." << std::endl;

	delete device;
	delete physicalDevice;
	delete instance;
}

int main()
{
	if (!InitializeVulkan())
	{
		return 1;
	}

	VkCommandBuffer* commands = new VkCommandBuffer[3];
	device->GetComputeCommand(commands, 3);
	device->FreeComputeCommand(commands, 3);

	Clear();

	return 0;
}
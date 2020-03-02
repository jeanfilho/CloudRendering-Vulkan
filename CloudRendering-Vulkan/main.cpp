#include "stdafx.h"

#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"

VulkanInstance* instance;
VulkanPhysicalDevice* physicalDevice;
VulkanDevice* device;
VulkanBuffer* buffer;

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
	std::cout << "Clearing API instances..." << std::endl;

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

	float* arr = new float[3];
	for (int i = 0; i < 3; i++)
	{
		arr[i] = (float)i;
	}


	buffer = new VulkanBuffer(device, arr, sizeof(float), 3);
	buffer->SetData();

	device->FreeComputeCommand(commands, 3);	

	delete[] arr;
	delete buffer;

	Clear();

	return 0;
}
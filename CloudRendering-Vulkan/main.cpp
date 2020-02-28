#include "stdafx.h"

#include "VulkanConfiguration.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"

VulkanInstance* pInstance;
VulkanPhysicalDevice* pPhysicalDevice;

int main()
{
	VulkanConfiguration config{};
	config.applicationName = "Cloud Renderer";
	config.applicationVersion = VK_MAKE_VERSION(0, 0, 0);

	pInstance = new VulkanInstance(config);
	pPhysicalDevice = VulkanPhysicalDevice::CreatePhysicalDevice(pInstance);

	delete pPhysicalDevice;
	delete pInstance;
	return 0;
}
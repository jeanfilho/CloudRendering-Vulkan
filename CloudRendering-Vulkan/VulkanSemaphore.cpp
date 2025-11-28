#include "stdafx.h"
#include "VulkanSemaphore.h"

#include "VulkanDevice.h"

VulkanSemaphore::VulkanSemaphore(VulkanDevice* device)
{
	m_device = device;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	ValidCheck(vkCreateSemaphore(m_device->GetDevice(), &semaphoreInfo, nullptr, &m_semaphore));
}

VulkanSemaphore::VulkanSemaphore(VulkanSemaphore&& other) noexcept
{
	m_device = std::move(other.m_device);
	m_semaphore = std::move(other.m_semaphore);

    other.m_device = VK_NULL_HANDLE;
    other.m_semaphore = VK_NULL_HANDLE;
}

VulkanSemaphore::VulkanSemaphore(const VulkanSemaphore&& other)
{
    m_device = other.m_device;
    m_semaphore = other.m_semaphore;
}

VulkanSemaphore::~VulkanSemaphore()
{
	if (m_semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(m_device->GetDevice(), m_semaphore, nullptr);
	}
}

VkSemaphore VulkanSemaphore::GetSemaphore()
{
	return m_semaphore;
}

#include "stdafx.h"

#include "VulkanInstance.h"
#include "VulkanBuffer.h"
#include "VulkanBufferView.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "VulkanSampler.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanImGUIRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanSemaphore.h"
#include "VulkanFence.h"
#include "VulkanDescriptorPool.h"

#include "RenderTechniquePT.h"
#include "RenderTechniqueSV.h"
#include "RenderTechniquePPM.h"
#include "RenderTechniquePPB.h"

#include "Grid3D.h"
#include "Tests.h"
#include "ImGUILayer.h"

//--------------------------------------------------------------
// Shader Resources
//--------------------------------------------------------------
ShadowVolumeProperties g_shadowVolumeProperties;
VulkanBuffer* g_shadowVolumePropertiesBuffer;

CameraProperties g_cameraProperties;
VulkanBuffer* g_cameraPropertiesBuffer;

CloudProperties g_cloudProperties;
VulkanBuffer* g_cloudPropertiesBuffer;

Parameters g_parameters;
VulkanBuffer* g_parametersBuffer;

PhotonMapProperties g_photonMapProperties;
VulkanBuffer* g_photonMapPropertiesBuffer;

PushConstants g_pushConstants;

//--------------------------------------------------------------
// Globals
//--------------------------------------------------------------
RenderTechniquePT* g_pathTracingTechnique;
RenderTechniquePPM* g_photonMappingTechnique;
RenderTechniqueSV* g_shadowVolumeTechnique;
RenderTechniquePPB* g_photonBeamsTechnique;

VulkanInstance* g_instance;
VulkanPhysicalDevice* g_physicalDevice;
VulkanDevice* g_device;
VulkanSurface* g_surface;
VulkanSwapchain* g_swapchain;
std::vector<VulkanImageView> g_swapchainImageViews;

VulkanCommandPool* g_graphicsCommandPool;
std::vector<VulkanFramebuffer*> g_framebuffers;

VulkanCommandPool* g_computeCommandPool;
VulkanDescriptorPool* g_computeDescriptorPool;

Grid3D<float>* g_cloudData;
VulkanImage* g_cloudImage;
VulkanImageView* g_cloudImageView;
VulkanSampler* g_cloudSampler;

VulkanImage* g_shadowVolumeImage;
VulkanImageView* g_shadowVolumeImageView;
VulkanSampler* g_shadowVolumeSampler;

std::vector<VulkanImage*> g_resultImages;
std::vector<VulkanImageView*> g_resultImageViews;

ImGUILayer* g_imguiLayer = nullptr;

std::vector<VulkanSemaphore> g_imageAvailableSemaphores;
std::vector<VulkanSemaphore> g_computeFinishedSemaphores;
std::vector<VulkanSemaphore> g_graphicsFinishedSemaphores;
std::vector<VulkanFence> g_inFlightFences;
std::vector<VkFence> g_imagesInFlight; //Vulkan type needed for function call in rendering loop
uint32_t g_currentFrameIdx = 0;
uint32_t g_swapchainImageIdx = 0;

bool g_framebufferResized = false;

GLFWwindow* g_window;

double g_renderStartTime = 0;
double g_previousTime = 0;
unsigned int g_framesInSecond = 0;

constexpr int MAX_FRAMES_IN_FLIGHT = 1;
const char* CLOUD_FILE_PATH = "../models/mycloud.xyz";

//----------------------------------------------------------------------
// UI
//----------------------------------------------------------------------

char g_UICloudFile[1024];
std::string g_UICurrentCloudFile = " ";
float g_UIPhaseG = g_parameters.GetPhaseG();
float g_UISecondsPerFrame = 0;
glm::vec2 g_UICameraRotate{ 0, 0 };
glm::vec3 g_UILightDirection = g_shadowVolumeProperties.GetLightDirection();
int g_UICurrentResolution = 0;
int g_UIPreviousResolution = g_UICurrentResolution;
const char* RESOLUTIONS_NAMES[] = { "800x600", "1920x1080" };
const glm::ivec2 RESOLUTIONS[] = { {800, 600}, {1920, 1080} };
float g_UIFov = 90.f;
RenderTechnique* g_currentTechnique = nullptr;

//----------------------------------------------------------------------
// Enums
//----------------------------------------------------------------------

enum class ERenderTechnique
{
	PathTracing = 0,
	PhotonMapping,
	PhotonBeams
};

//----------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------
void UpdateTime()
{
	g_pushConstants.time = glfwGetTime();
	if (g_pushConstants.time - g_previousTime >= 1.0)
	{
		g_UISecondsPerFrame = static_cast<float>(1000.0 / double(g_framesInSecond));
		g_framesInSecond = 0;
		g_previousTime += 1.0;
	}
}

template<typename T>
void SetCloudProperties(Grid3D<T>* grid)
{
	glm::vec3 cloudSize{
		grid->GetVoxelSize().x * grid->GetVoxelCount().x * g_cloudProperties.baseScaling,
		grid->GetVoxelSize().y * grid->GetVoxelCount().y * g_cloudProperties.baseScaling,
		grid->GetVoxelSize().z * grid->GetVoxelCount().z * g_cloudProperties.baseScaling };

	g_cloudProperties.maxExtinction = std::max(grid->GetMajorant(), 0.01f);
	g_cloudProperties.voxelCount = glm::uvec4(grid->GetVoxelCount(), 0);
	g_cloudProperties.bounds[0] = glm::vec4(
		-cloudSize.x / 2,
		-cloudSize.y / 2,
		0,
		0
	);
	g_cloudProperties.bounds[1] = -g_cloudProperties.bounds[0] + glm::vec4(0, 0, cloudSize.z, 0);

	g_photonMapProperties.SetBounds(g_cloudProperties.bounds);
}


bool LoadCloudFile(const std::string filename)
{
	std::cout << "Loading cloud file...";


	Grid3D<float>* cloudData = Grid3D<float>::Load("../models/" + filename);
	if (!cloudData)
	{
		std::cout << " ERROR: Failed to load file \"" + filename + "\" in models folder" << std::endl;
		return false;
	}

	if (g_cloudData)
	{
		delete g_cloudData;
	}
	g_cloudData = cloudData;

	g_UICurrentCloudFile = filename;

	SetCloudProperties(g_cloudData);

	std::cout << "OK" << std::endl;
	return true;
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	std::cout << "Button pressed";
}

void RecordComputeCommands(uint32_t imageIndex)
{
	VkCommandBuffer& commandBuffer = g_computeCommandPool->GetCommandBuffers()[g_swapchainImageIdx];
	VkImage swapchainImage = g_swapchain->GetSwapchainImages()[imageIndex];

	// Clear existing commands
	vkResetCommandBuffer(commandBuffer, 0);

	// Start recording commands
    VkCommandBufferBeginInfo beginInfo = initializers::CommandBufferBeginInfo();
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Record technique commands
	g_currentTechnique->RecordDrawCommands(commandBuffer, imageIndex);

	// End recording
	vkEndCommandBuffer(commandBuffer);
}

void RecordImGUICommands(uint32_t imageIndex)
{
	VkCommandBuffer commandBuffer = g_graphicsCommandPool->GetCommandBuffers()[g_swapchainImageIdx];
	VkImage swapchainImage = g_swapchain->GetSwapchainImages()[imageIndex];

	vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo = initializers::CommandBufferBeginInfo();
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkClearValue clearValue{};
	clearValue.color = VkClearColorValue();
	clearValue.depthStencil = VkClearDepthStencilValue();

	VkRenderPassBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.renderPass = g_imguiLayer->GetRenderPass()->GetRenderPass();
	info.framebuffer = g_framebuffers[imageIndex]->GetFramebuffer();
	info.renderArea.extent.width = g_cameraProperties.GetWidth();
	info.renderArea.extent.height = g_cameraProperties.GetHeight();
	info.clearValueCount = 1;
	info.pClearValues = &clearValue;
	vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	ValidCheck(vkEndCommandBuffer(commandBuffer));
}

void SetRenderTechnique(ERenderTechnique renderTechnique)
{
	g_photonMappingTechnique->FreeResources();
	g_photonBeamsTechnique->FreeResources();

	switch (renderTechnique)
	{
	case ERenderTechnique::PhotonMapping:
		g_currentTechnique = g_photonMappingTechnique;
		g_photonMappingTechnique->AllocateResources(g_photonMapPropertiesBuffer);
		break;
	case ERenderTechnique::PathTracing:
		g_currentTechnique = g_pathTracingTechnique;
		break;
	case ERenderTechnique::PhotonBeams:
		g_currentTechnique = g_photonBeamsTechnique;
		g_photonBeamsTechnique->AllocateResources();
		for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
		{
			g_photonBeamsTechnique->UpdatePhotonMapProperties(g_photonMapPropertiesBuffer, i);
		}
		g_photonBeamsTechnique->UpdateDescriptorSets();
		break;
	}
}

void UpdateShadowVolume()
{
	g_photonMapProperties.lightDirection = glm::normalize(glm::vec4(g_UILightDirection, 0));
	if (g_currentTechnique == g_photonBeamsTechnique)
	{
		g_shadowVolumeProperties.SetLightDirection(g_cameraProperties.GetFwd());
	}
	else
	{
		g_shadowVolumeProperties.SetLightDirection(g_UILightDirection);
	}

	g_shadowVolumeProperties.SetOrigin(g_cloudProperties.bounds[0], g_cloudProperties.bounds[1]);
	g_shadowVolumePropertiesBuffer->SetData();

	// Update shadow volume descriptor set
	auto bufferInfo = initializers::DescriptorBufferInfo(g_shadowVolumePropertiesBuffer->GetBuffer(), 0, g_shadowVolumePropertiesBuffer->GetSize());
	g_shadowVolumeTechnique->QueueUpdateShadowVolume(bufferInfo, 0);
	g_shadowVolumeTechnique->UpdateDescriptorSets();

	// Wait until any calculations are done
	vkQueueWaitIdle(g_device->GetComputeQueue());

	// Send commands for updating volume
	VkCommandBuffer commandBuffer = utilities::BeginSingleTimeCommands(g_device, g_computeCommandPool);
	g_shadowVolumeTechnique->RecordDrawCommands(commandBuffer, 0);
	utilities::EndSingleTimeCommands(g_device, g_computeCommandPool, commandBuffer);

	// Update render technique descriptor set
	for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
	{
		g_pathTracingTechnique->QueueUpdateShadowVolume(bufferInfo, i);
		g_photonMappingTechnique->QueueUpdateShadowVolume(bufferInfo, i);
		g_photonBeamsTechnique->QueueUpdateShadowVolume(bufferInfo, i);
	}
	g_pathTracingTechnique->UpdateDescriptorSets();
	g_photonMappingTechnique->UpdateDescriptorSets();
	g_photonBeamsTechnique->UpdateDescriptorSets();

	g_pushConstants.frameCount = 1;
	g_renderStartTime = glfwGetTime();
}

void UpdateCloudData()
{
	vkDeviceWaitIdle(g_device->GetDevice());

	if (g_cloudImage)
	{
		delete g_cloudImage;
		delete g_cloudImageView;
		delete g_cloudSampler;
	}

	g_cloudImage = new VulkanImage(
		g_device,
		VK_FORMAT_R32_SFLOAT,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		static_cast<uint32_t>(g_cloudProperties.voxelCount.x),
		static_cast<uint32_t>(g_cloudProperties.voxelCount.y),
		static_cast<uint32_t>(g_cloudProperties.voxelCount.z));
	g_cloudImageView = new VulkanImageView(g_device, g_cloudImage);
	g_cloudSampler = new VulkanSampler(g_device);

	{
		VulkanBuffer stagingBuffer(g_device, g_cloudData->GetData(), g_cloudData->GetElementSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, g_cloudData->GetSize());
		stagingBuffer.SetData();

		VkCommandBuffer commandBuffer = utilities::BeginSingleTimeCommands(g_device, g_computeCommandPool);
		utilities::CmdTransitionImageLayout(commandBuffer, g_cloudImage->GetImage(), g_cloudImage->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		utilities::CmdCopyBufferToImage(commandBuffer, stagingBuffer.GetBuffer(), g_cloudImage->GetImage(), g_cloudImage->GetExtent());
		utilities::CmdTransitionImageLayout(commandBuffer, g_cloudImage->GetImage(), g_cloudImage->GetFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		utilities::EndSingleTimeCommands(g_device, g_computeCommandPool, commandBuffer);

		vkDeviceWaitIdle(g_device->GetDevice());
		{
			g_cloudPropertiesBuffer->SetData();
			g_photonMapPropertiesBuffer->SetData();

			auto cloudBufferInfo = initializers::DescriptorBufferInfo(g_cloudPropertiesBuffer->GetBuffer(), 0, g_cloudPropertiesBuffer->GetSize());
			auto cloudImageInfo = initializers::DescriptorImageInfo(g_cloudSampler->GetSampler(), g_cloudImageView->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
			{
				g_pathTracingTechnique->QueueUpdateCloudDataSampler(cloudImageInfo, i);
				g_pathTracingTechnique->QueueUpdateCloudData(cloudBufferInfo, i);

				g_photonMappingTechnique->QueueUpdateCloudDataSampler(cloudImageInfo, i);
				g_photonMappingTechnique->QueueUpdateCloudData(cloudBufferInfo, i);

				g_photonBeamsTechnique->QueueUpdateCloudDataSampler(cloudImageInfo, i);
				g_photonBeamsTechnique->QueueUpdateCloudData(cloudBufferInfo, i);
			}
			g_pathTracingTechnique->UpdateDescriptorSets();
			g_photonMappingTechnique->UpdateDescriptorSets();
			g_photonBeamsTechnique->UpdateDescriptorSets();

			g_shadowVolumeTechnique->QueueUpdateCloudData(cloudBufferInfo, 0);
			g_shadowVolumeTechnique->QueueUpdateCloudDataSampler(cloudImageInfo, 0);
			g_shadowVolumeTechnique->UpdateDescriptorSets();
		}

		g_photonMappingTechnique->FreeResources();
		g_photonMappingTechnique->AllocateResources(g_photonMapPropertiesBuffer);

		for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
		{
			g_photonBeamsTechnique->UpdatePhotonMapProperties(g_photonMapPropertiesBuffer, i);
		}

		UpdateShadowVolume();
	}
}

void ClearSwapchain()
{
	vkDeviceWaitIdle(g_device->GetDevice());

	std::cout << "Clearing swapchain...";

	// Recreate result images. They should match the resolution of the screen
	g_pathTracingTechnique->ClearFrameReferences();
	g_photonMappingTechnique->ClearFrameReferences();
	g_photonBeamsTechnique->ClearFrameReferences();

	// Recreate framebuffers
	for (auto& framebuffer : g_framebuffers)
	{
		delete framebuffer;
		framebuffer = nullptr;
	}

	// Recreate swapchain image views and swapchain
	g_swapchainImageViews.clear();
	delete g_swapchain;
	g_swapchain = nullptr;

	// Recreate result images
	for (unsigned int i = 0; i < g_resultImages.size(); i++)
	{
		delete g_resultImages[i];
		delete g_resultImageViews[i];

		g_resultImages[i] = nullptr;
		g_resultImageViews[i] = nullptr;
	}

	std::cout << "OK" << std::endl;
}

void CreateSwapchain()
{
	vkDeviceWaitIdle(g_device->GetDevice());

	int width = 0, height = 0;
	glfwGetFramebufferSize(g_window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(g_window, &width, &height);
		glfwWaitEvents();
	}
	if (g_swapchain)
	{
		vkDeviceWaitIdle(g_device->GetDevice());
		ClearSwapchain();
	}

	// Swapchain
	g_swapchain = new VulkanSwapchain(g_device, g_window);

	// Swapchain image views
	g_swapchainImageViews.reserve(g_swapchain->GetImageCount());
	for (const VkImage& image : g_swapchain->GetSwapchainImages())
	{
		g_swapchainImageViews.emplace_back(g_device, image, g_swapchain->GetImageFormat(), VK_IMAGE_VIEW_TYPE_2D);
	}

	// Compute result image and view
	for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
	{
		g_resultImages.push_back(new VulkanImage(g_device,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			static_cast<uint32_t>(g_cameraProperties.GetWidth()),
			static_cast<uint32_t>(g_cameraProperties.GetHeight())));
		g_resultImageViews.push_back(new VulkanImageView(g_device, g_resultImages[i]));
	}
	VkCommandBuffer commandBuffer = utilities::BeginSingleTimeCommands(g_device, g_computeCommandPool);

	// Transition to general since they will be written to in the rendering techniques
	for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
	{
		utilities::CmdTransitionImageLayout(commandBuffer, g_resultImages[i]->GetImage(), g_resultImages[i]->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	}
	utilities::EndSingleTimeCommands(g_device, g_computeCommandPool, commandBuffer);

	// Create framebuffers for ImGUI if already present
	if (g_imguiLayer)
	{
		g_framebuffers.resize(g_swapchainImageViews.size());
		for (size_t i = 0; i < g_framebuffers.size(); i++)
		{
			g_framebuffers[i] = new VulkanFramebuffer(g_device, g_imguiLayer->GetRenderPass(), &g_swapchainImageViews[i], g_swapchain);
		}
	}
}

void Clear()
{
	delete g_imguiLayer;

	ClearSwapchain();

	std::cout << "Clearing allocations...";
	g_inFlightFences.clear();
	g_graphicsFinishedSemaphores.clear();
	g_computeFinishedSemaphores.clear();
	g_imageAvailableSemaphores.clear();
    g_imagesInFlight.clear();

	// Graphics
	delete g_graphicsCommandPool;

	// Shadow Volume
	delete g_shadowVolumeTechnique;
	delete g_shadowVolumePropertiesBuffer;
	delete g_shadowVolumeImageView;
	delete g_shadowVolumeImage;
	delete g_shadowVolumeSampler;

	// Cloud Memory Allocations
	delete g_cloudImageView;
	delete g_cloudSampler;
	delete g_cloudImage;
	delete g_cameraPropertiesBuffer;
	delete g_cloudPropertiesBuffer;
	delete g_parametersBuffer;
	delete g_photonMapPropertiesBuffer;

	// Compute Resources
	delete g_pathTracingTechnique;
	delete g_photonMappingTechnique;
	delete g_photonBeamsTechnique;
	delete g_computeCommandPool;
	delete g_computeDescriptorPool;

	// Vulkan General Resources
	delete g_device;
	delete g_physicalDevice;
	delete g_surface;
	delete g_instance;

	glfwDestroyWindow(g_window);
	glfwTerminate();

	std::cout << "OK" << std::endl;
}

void UpdateUI()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	ImGui::Begin("Rendering Stats");
	{
		ImGui::Text("FrameCount: %i", g_pushConstants.frameCount);
		ImGui::Text("Elapsed time: %.2f", g_pushConstants.time - g_renderStartTime);
		ImGui::Text("ms/frame: %.2f", g_UISecondsPerFrame);
	}
	ImGui::End();


	ImGui::Begin("Cloud File");
	{
		ImGui::Text("Current File: %s", g_UICurrentCloudFile.c_str());
		ImGui::InputText("Filename", g_UICloudFile, 1024);
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			if (LoadCloudFile(std::string(g_UICloudFile)))
			{
				UpdateCloudData();
			}
		}
	}
	ImGui::End();

	ImGui::Begin("Settings");
	{
		ImGui::Text("Parameters");
		ImGui::SliderFloat("Henyey-Greenstein G ", &g_UIPhaseG, -0.95f, 0.95f);

		ImGui::Separator();

		ImGui::Text("Camera");
		ImGui::InputFloat3("Position ", &g_cameraProperties.position[0]);
		ImGui::InputFloat2("Rotation ", &g_UICameraRotate[0]);
		ImGui::InputFloat("FOV", &g_UIFov);
		ImGui::Combo("Resolution", &g_UICurrentResolution, RESOLUTIONS_NAMES, IM_ARRAYSIZE(RESOLUTIONS_NAMES));

		ImGui::Separator();

		ImGui::Text("Lighting");
		ImGui::InputFloat3("Direction ", &g_UILightDirection[0]);
		ImGui::SliderFloat("Intensity ", &g_parameters.lightIntensity, 0, 10);

		ImGui::Separator();
		ImGui::Text("Cloud");
		ImGui::SliderFloat("Density", &g_cloudProperties.densityScaling, 0, 1000);

		if (ImGui::Button("Apply"))
		{

			if (g_UIPreviousResolution != g_UICurrentResolution)
			{
				g_UIPreviousResolution = g_UICurrentResolution;
				glfwSetWindowSize(g_window, RESOLUTIONS[g_UICurrentResolution].x, RESOLUTIONS[g_UICurrentResolution].y);

				ClearSwapchain();
				CreateSwapchain();
			}

			// Set frame references again
			g_pathTracingTechnique->SetFrameReferences(g_resultImages, g_resultImageViews, g_swapchain);
			g_photonMappingTechnique->SetFrameReferences(g_resultImages, g_resultImageViews, g_swapchain);
			g_photonBeamsTechnique->SetFrameReferences(g_resultImages, g_resultImageViews, g_swapchain);

			// Recreate command buffers
			g_computeCommandPool->AllocateCommandBuffers(g_swapchain->GetImageCount());
			g_graphicsCommandPool->AllocateCommandBuffers(g_swapchain->GetImageCount());

			// Update data in memory
			g_parameters.SetPhaseG(g_UIPhaseG);
			g_cameraProperties.SetFOV(g_UIFov);
			g_cameraProperties.SetRotation(g_UICameraRotate);

			g_parametersBuffer->SetData();
			g_cameraPropertiesBuffer->SetData();
			g_cloudPropertiesBuffer->SetData();

			// Wait for all frames to finish
			vkDeviceWaitIdle(g_device->GetDevice());

			// Update sets in gpu
			auto parametersInfo = initializers::DescriptorBufferInfo(g_parametersBuffer->GetBuffer(), 0, g_parametersBuffer->GetSize());
			auto cameraPropertiesInfo = initializers::DescriptorBufferInfo(g_cameraPropertiesBuffer->GetBuffer(), 0, g_cameraPropertiesBuffer->GetSize());
			auto cloudBufferInfo = initializers::DescriptorBufferInfo(g_cloudPropertiesBuffer->GetBuffer(), 0, g_cloudPropertiesBuffer->GetSize());

			for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
			{
				g_photonMappingTechnique->QueueUpdateCameraProperties(cameraPropertiesInfo, i);
				g_photonMappingTechnique->QueueUpdateParameters(parametersInfo, i);
				g_photonMappingTechnique->QueueUpdateCloudData(cloudBufferInfo, i);

				g_pathTracingTechnique->QueueUpdateCameraProperties(cameraPropertiesInfo, i);
				g_pathTracingTechnique->QueueUpdateParameters(parametersInfo, i);
				g_pathTracingTechnique->QueueUpdateCloudData(cloudBufferInfo, i);
			}
			g_photonMappingTechnique->UpdateDescriptorSets();
			g_pathTracingTechnique->UpdateDescriptorSets();

			UpdateShadowVolume();
		}
	}
	ImGui::End();

	ImGui::Render();
}

void DrawFrame()
{
	// Wait for queue to finish if it is still running, and restore fence to original state
	vkWaitForFences(g_device->GetDevice(), 1, &g_inFlightFences[g_currentFrameIdx].GetFence(), VK_TRUE, UINT64_MAX);

	// Acquire image from swapchain
	uint32_t imageIndex;
	ValidCheck(vkAcquireNextImageKHR(g_device->GetDevice(), g_swapchain->GetSwapchain(), UINT64_MAX, g_imageAvailableSemaphores[g_swapchainImageIdx].GetSemaphore(), VK_NULL_HANDLE, &imageIndex));

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (g_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(g_device->GetDevice(), 1, &g_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	// Reset current fence (mark as unsignaled)
	vkResetFences(g_device->GetDevice(), 1, &g_inFlightFences[g_currentFrameIdx].GetFence());

	// Mark the image as now being in use by this frame
	g_imagesInFlight[imageIndex] = g_inFlightFences[g_currentFrameIdx].GetFence();

	// Record commands
	RecordComputeCommands(imageIndex);

	// Submit compute command buffer to queue
	{
		std::vector<VkCommandBuffer> commandBuffers{ g_computeCommandPool->GetCommandBuffers()[g_swapchainImageIdx]};
		VkSemaphore waitSemaphores[] = { g_imageAvailableSemaphores[g_swapchainImageIdx].GetSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };
		VkSemaphore signalSemaphores[] = { g_computeFinishedSemaphores[g_swapchainImageIdx].GetSemaphore() };

		VkSubmitInfo computeSubmit = initializers::SubmitInfo();
		computeSubmit.pWaitSemaphores = waitSemaphores;
		computeSubmit.waitSemaphoreCount = 1;
		computeSubmit.pWaitDstStageMask = waitStages;
		computeSubmit.pCommandBuffers = commandBuffers.data();
		computeSubmit.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
		computeSubmit.pSignalSemaphores = signalSemaphores;
		computeSubmit.signalSemaphoreCount = 1;

		// Submit commands to compute queue and signal the inFlightFence when finished
		ValidCheck(vkQueueSubmit(g_device->GetComputeQueue(), 1, &computeSubmit, VK_NULL_HANDLE));
	}

	// Wait on compute to finish before submitting the graphics queue
	RecordImGUICommands(imageIndex);

	// Submit graphics command buffer to graphics queue, wait on compute completion
	{
		std::vector<VkCommandBuffer> commandBuffers{ g_graphicsCommandPool->GetCommandBuffers()[g_swapchainImageIdx] };
		VkSemaphore waitSemaphores[] = { g_computeFinishedSemaphores[g_swapchainImageIdx].GetSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
		VkSemaphore signalSemaphores[] = { g_graphicsFinishedSemaphores[g_swapchainImageIdx].GetSemaphore() };

		VkPipelineStageFlags waitStagesGraphics[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo graphicsSubmit = initializers::SubmitInfo();
		graphicsSubmit.waitSemaphoreCount = 1;
		graphicsSubmit.pWaitSemaphores = waitSemaphores;
		graphicsSubmit.pWaitDstStageMask = waitStagesGraphics;
		graphicsSubmit.pCommandBuffers = commandBuffers.data(); // graphics CB
		graphicsSubmit.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
		graphicsSubmit.signalSemaphoreCount = 1;
		graphicsSubmit.pSignalSemaphores = signalSemaphores;

		ValidCheck(vkQueueSubmit(g_device->GetGraphicsQueue(), 1, &graphicsSubmit, g_imagesInFlight[imageIndex]));
	}

	// Present image
	{
		VkSwapchainKHR swapchains[] = { g_swapchain->GetSwapchain() };
		VkSemaphore waitSemaphores[] = { g_graphicsFinishedSemaphores[g_swapchainImageIdx].GetSemaphore() };

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		vkQueuePresentKHR(g_device->GetPresentQueue(), &presentInfo);
	}

	// Advance to next frame idx and swapchain image
	g_currentFrameIdx = (g_currentFrameIdx + 1) % MAX_FRAMES_IN_FLIGHT;
    g_swapchainImageIdx = (g_swapchainImageIdx + 1) % g_swapchain->GetImageCount();
}

void RenderLoop()
{
	std::cout << "Render Loop started" << std::endl;
	while (!glfwWindowShouldClose(g_window))
	{
		glfwPollEvents();
		UpdateTime();

		g_pushConstants.seed = std::rand();

		if (!glfwGetWindowAttrib(g_window, GLFW_ICONIFIED))
		{
			UpdateUI();
			DrawFrame();
			g_pushConstants.frameCount++;
			g_framesInSecond++;
		}
	}

	std::cout << "Render Loop stopped" << std::endl;
	vkDeviceWaitIdle(g_device->GetDevice());
	std::cout << "Device Finished" << std::endl;
}

void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	g_framebufferResized = true;
	g_cameraProperties.SetResolution(width, height);
}


void InitializeImGUI()
{
	ImGui_ImplVulkan_InitInfo initInfo{};

	initInfo.Instance = g_instance->GetInstance();
	initInfo.PhysicalDevice = g_physicalDevice->GetPhysicaDevice();
	initInfo.Device = g_device->GetDevice();
	initInfo.QueueFamily = g_physicalDevice->GetQueueFamilyIndices().graphicsFamily;
	initInfo.Queue = g_device->GetGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE; //Unused
	initInfo.DescriptorPool = VK_NULL_HANDLE; // Created by imguiLayer;
	initInfo.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
	initInfo.Allocator = nullptr; //Unused
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = static_cast<uint32_t>(g_swapchain->GetSwapchainImages().size());
	initInfo.CheckVkResultFn = ValidCheck;

	g_imguiLayer = new ImGUILayer(g_window, g_device, g_swapchain, initInfo);
}

bool InitializeVulkan()
{
	std::cout << "Initializing Vulkan... ";

	VulkanConfiguration config{};
	config.applicationName = "Cloud Renderer";
	config.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	// Instance
	g_instance = new VulkanInstance(config);

	// Surface
	g_surface = new VulkanSurface(g_instance, g_window);

	// Physical Device
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	g_physicalDevice = VulkanPhysicalDevice::CreatePhysicalDevice(g_instance, g_surface, deviceExtensions);
	if (!g_physicalDevice)
	{
		throw std::runtime_error("Failed to create Physical Device");
		return false;
	}

	// Device
	g_device = new VulkanDevice(g_instance, g_surface, g_physicalDevice);

	// Command Pool
	g_computeCommandPool = new VulkanCommandPool(g_device, g_physicalDevice->GetQueueFamilyIndices().computeFamily);
	g_graphicsCommandPool = new VulkanCommandPool(g_device, g_physicalDevice->GetQueueFamilyIndices().graphicsFamily);

	// Buffers & Images
	g_cameraPropertiesBuffer = new VulkanBuffer(g_device, &g_cameraProperties, sizeof(CameraProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_cloudPropertiesBuffer = new VulkanBuffer(g_device, &g_cloudProperties, sizeof(CloudProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_parametersBuffer = new VulkanBuffer(g_device, &g_parameters, sizeof(Parameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_photonMapPropertiesBuffer = new VulkanBuffer(g_device, &g_photonMapProperties, sizeof(PhotonMapProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	g_shadowVolumePropertiesBuffer = new VulkanBuffer(g_device, &g_shadowVolumeProperties, sizeof(ShadowVolumeProperties), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_shadowVolumeImage = new VulkanImage(g_device, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, g_shadowVolumeProperties.voxelAxisCount, g_shadowVolumeProperties.voxelAxisCount, g_shadowVolumeProperties.voxelAxisCount);
	g_shadowVolumeImageView = new VulkanImageView(g_device, g_shadowVolumeImage);
	g_shadowVolumeSampler = new VulkanSampler(g_device);

	g_cameraPropertiesBuffer->SetData();
	g_parametersBuffer->SetData();
	g_photonMapPropertiesBuffer->SetData();

	// Render Techniques
	g_shadowVolumeTechnique = new RenderTechniqueSV(g_device, &g_shadowVolumeProperties, &g_pushConstants);
	g_pathTracingTechnique = new RenderTechniquePT(g_device, g_swapchain, &g_cameraProperties, &g_pushConstants);
	g_photonMappingTechnique = new RenderTechniquePPM(g_device, g_swapchain, &g_cameraProperties, &g_photonMapProperties, &g_pushConstants, 10);
	g_photonBeamsTechnique = new RenderTechniquePPB(g_device, &g_pushConstants, &g_cameraProperties, 200);

	// Create swapchain
	CreateSwapchain();
	if (!g_imguiLayer)
	{
		InitializeImGUI();
	}

	// Compute Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
	{
		g_pathTracingTechnique->GetDescriptorPoolSizes(poolSizes);
        g_photonMappingTechnique->GetDescriptorPoolSizes(poolSizes);
        g_photonBeamsTechnique->GetDescriptorPoolSizes(poolSizes);		
	}
    g_shadowVolumeTechnique->GetDescriptorPoolSizes(poolSizes);

	uint32_t requiredSets = g_shadowVolumeTechnique->GetRequiredSetCount() +
		(g_pathTracingTechnique->GetRequiredSetCount() + g_photonMappingTechnique->GetRequiredSetCount() + g_photonBeamsTechnique->GetRequiredSetCount()) * g_swapchain->GetImageCount();
	g_computeDescriptorPool = new VulkanDescriptorPool(g_device, poolSizes, requiredSets);

	g_computeDescriptorPool->AllocateSets(g_pathTracingTechnique, g_swapchain->GetImageCount());
	g_computeDescriptorPool->AllocateSets(g_photonMappingTechnique, g_swapchain->GetImageCount());
	g_computeDescriptorPool->AllocateSets(g_photonBeamsTechnique, g_swapchain->GetImageCount());
	g_computeDescriptorPool->AllocateSets(g_shadowVolumeTechnique, 1);

	g_pathTracingTechnique->SetFrameReferences(g_resultImages, g_resultImageViews, g_swapchain);
	g_photonMappingTechnique->SetFrameReferences(g_resultImages, g_resultImageViews, g_swapchain);
	g_photonBeamsTechnique->SetFrameReferences(g_resultImages, g_resultImageViews, g_swapchain);

	// Recreate command buffers
	g_computeCommandPool->AllocateCommandBuffers(g_swapchain->GetImageCount());
	g_graphicsCommandPool->AllocateCommandBuffers(g_swapchain->GetImageCount());

	// Set shadow volume output image
	std::vector<VulkanImage*> shadowImg{ g_shadowVolumeImage };
	std::vector<VulkanImageView*> shadowView{ g_shadowVolumeImageView };
	g_shadowVolumeTechnique->SetFrameReferences(shadowImg, shadowView, nullptr);

	// Transfer cloud data to device image and make it readable by the shader
	UpdateCloudData();

	// Update descriptor sets
	auto parameterInfo = initializers::DescriptorBufferInfo(g_parametersBuffer->GetBuffer(), 0, g_parametersBuffer->GetSize());
	auto cameraPropertiesInfo = initializers::DescriptorBufferInfo(g_cameraPropertiesBuffer->GetBuffer(), 0, g_cameraPropertiesBuffer->GetSize());
	auto shadowImageInfo = initializers::DescriptorImageInfo(g_shadowVolumeSampler->GetSampler(), g_shadowVolumeImageView->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	for (unsigned int i = 0; i < g_swapchain->GetImageCount(); i++)
	{
		g_pathTracingTechnique->QueueUpdateParameters(parameterInfo, i);
		g_pathTracingTechnique->QueueUpdateCameraProperties(cameraPropertiesInfo, i);
		g_pathTracingTechnique->QueueUpdateShadowVolumeSampler(shadowImageInfo, i);

		g_photonMappingTechnique->QueueUpdateParameters(parameterInfo, i);
		g_photonMappingTechnique->QueueUpdateCameraProperties(cameraPropertiesInfo, i);
		g_photonMappingTechnique->QueueUpdateShadowVolumeSampler(shadowImageInfo, i);

		g_photonBeamsTechnique->QueueUpdateParameters(parameterInfo, i);
		g_photonBeamsTechnique->QueueUpdateCameraProperties(cameraPropertiesInfo, i);
		g_photonBeamsTechnique->QueueUpdateShadowVolumeSampler(shadowImageInfo, i);
	}
	g_photonBeamsTechnique->UpdateDescriptorSets();
	g_pathTracingTechnique->UpdateDescriptorSets();
	g_photonMappingTechnique->UpdateDescriptorSets();

	shadowImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	g_shadowVolumeTechnique->QueueUpdateShadowVolumeSampler(shadowImageInfo, 0);
	g_shadowVolumeTechnique->UpdateDescriptorSets();

	// Create framebuffers for ImGUI
	g_framebuffers.resize(g_swapchainImageViews.size());
	for (size_t i = 0; i < g_framebuffers.size(); i++)
	{
		g_framebuffers[i] = new VulkanFramebuffer(g_device, g_imguiLayer->GetRenderPass(), &g_swapchainImageViews[i], g_swapchain);
	}

	// Semaphores (GPU-GPU) and Fences (CPU-GPU)
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		g_inFlightFences.emplace_back(g_device);
	}

	for(size_t i = 0; i < g_swapchain->GetImageCount(); i++)
	{
		g_graphicsFinishedSemaphores.emplace_back(g_device);
		g_computeFinishedSemaphores.emplace_back(g_device);
		g_imageAvailableSemaphores.emplace_back(g_device);
    }

	g_imagesInFlight.resize(g_swapchain->GetSwapchainImages().size(), VK_NULL_HANDLE);

	std::cout << "OK" << std::endl;

	return true;
}

bool InitializeGLFW()
{
	std::cout << "Initializing GLFW... ";

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	g_window = glfwCreateWindow(g_cameraProperties.GetWidth(), g_cameraProperties.GetHeight(), "CloudRenderer", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(g_window, FramebufferResizeCallback);

	std::cout << "OK" << std::endl;
	return true;
}

void StartSimulation(ERenderTechnique technique)
{
	// Create default data
	g_cloudData = new Grid3D<float>(100, 100, 100, .01, .01, .01);
	SetCloudProperties(g_cloudData);
	g_cameraProperties.SetFOV(g_UIFov);

	// Initialize Framework
	InitializeGLFW();
	InitializeVulkan();

	SetRenderTechnique(technique);

	LoadCloudFile(CLOUD_FILE_PATH);
	UpdateCloudData();

	RenderLoop();
}

int main()
{
	// Seed random
	std::srand(0);

    StartSimulation(ERenderTechnique::PathTracing);

	Clear();

	delete g_cloudData;

	return 0;
}
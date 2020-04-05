#include "stdafx.h"
#include "ImGUILayer.h"

ImGUILayer::ImGUILayer(GLFWwindow* window, ImGui_ImplVulkan_InitInfo& initInfo, VkRenderPass renderPass)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(window, true);



	ImGui_ImplVulkan_Init(&initInfo, renderPass);
}

ImGUILayer::~ImGUILayer()
{
	ImGui::DestroyContext();
}

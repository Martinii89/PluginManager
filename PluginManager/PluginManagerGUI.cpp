#include "PluginManager.h"


void PluginManager::Render()
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(55 + 250 + 55 + 250 + 80, 600), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::Begin(menuTitle.c_str(), &isWindowOpen, ImGuiWindowFlags_NoCollapse);
	allPluginsVectorMutex.lock();

	ImGui::BeginChild("#LoadedPluginsTab", ImVec2(55 + 250 + 55 + 250, -ImGui::GetFrameHeightWithSpacing()));
	ImGui::Columns(4, "plugins"); // 4-ways, with border
	ImGui::SetColumnWidth(0, 55);
	ImGui::SetColumnWidth(1, 250);
	ImGui::SetColumnWidth(2, 55);
	ImGui::SetColumnWidth(3, 250);
	ImGui::Separator();
	ImGui::Text("Loaded"); ImGui::NextColumn();
	ImGui::Text("Plugin name"); ImGui::NextColumn();
	ImGui::Text("Version"); ImGui::NextColumn();
	ImGui::Text("Filename"); ImGui::NextColumn();
	ImGui::Separator();
	for (auto it = allPlugins.begin(); it != allPlugins.end(); it++) {
		
		bool isLoaded = it->second.loaded;
		
		bool changed = ImGui::Checkbox(("##" + it->first).c_str(), &isLoaded); ImGui::NextColumn();
		
		ImGui::Text(it->second.pluginName.c_str()); ImGui::NextColumn();
		ImGui::Text(it->second.version.c_str()); ImGui::NextColumn();
		ImGui::Text(it->second.dllName.c_str()); ImGui::NextColumn();
		ImGui::Separator();
		if (changed)
		{
			std::string command = isLoaded ? "load" : "unload"; //User pressed box, so we need to do what result of bool is

			//Add sleep or we might get a deadlock ("plugin" hook also locks allPluginsVectorMutex)
			cvarManager->executeCommand("sleep 1; plugin " + command + " " + it->second.dllName.substr(0, it->second.dllName.rfind('.')) + "; writeplugins");
		}
	}

	allPluginsVectorMutex.unlock();
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("#PluginsOptions");
	if (ImGui::Button("Install from ZIP"))
	{

	}
	ImGui::Button("Install from BakkesPlugins");
	ImGui::EndChild();

	ImGui::End();

	if (!isWindowOpen)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

std::string PluginManager::GetMenuName()
{
	return "pluginmanager";
}

std::string PluginManager::GetMenuTitle()
{
	return menuTitle;
}

void PluginManager::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool PluginManager::ShouldBlockInput()
{
	return true;
}

bool PluginManager::IsActiveOverlay()
{
	return false;
}

void PluginManager::OnOpen()
{
	isWindowOpen = true;
}

void PluginManager::OnClose()
{
	isWindowOpen = false;
}
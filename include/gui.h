#ifndef GUI_H
#define GUI_H

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <nfd/nfd.h>
//#include <scene_graph.h>

bool animationPlaying = false;
double previousTimeFPS;
int frameCount = 0;
int fps = 0;

void playAnimationButton()
{
	ImGui::Begin("ImGui Window");
	if (ImGui::Button("Play Animation")) {
		animationPlaying = !animationPlaying;
	}
	ImGui::End();
}

void DrawTree(const SceneNode* node)
{
	ImGui::TreeNode(node->name);

		for (int i = 0; i < node->numChildren; ++i)
			DrawTree(node->children[i]);

	for (const auto& child : node.children)
		
}

void DrawRoot(const RootSceneNode& Node) {
	for (const auto& child : node.children)
		DrawTree(child);
}

void LoadFile()
{
	ImGui::Begin("File");
	if (ImGui::Button("LoadFile")) {
		nfdchar_t* outPath = NULL;
		nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

		if (result == NFD_OKAY) {
			puts("Success!");
			puts(outPath);
			std::cout << "Path: " << outPath << std::endl;
			free(outPath);
		}
		else if (result == NFD_CANCEL) {
			puts("User pressed cancel.");
		}
		else {
			printf("Error: %s\n", NFD_GetError());
		}
	}
	ImGui::End();
}

void SceneWindow() {
	bool my_tool_active = true;
	// Create a window called "My First Tool", with a menu bar.
	ImGui::Begin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) {

				nfdchar_t* outPath = NULL;
				nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

				if (result == NFD_OKAY) {

					puts("Success!");
					puts(outPath);
					std::cout << "Path: " << outPath << std::endl;


					free(outPath);
				}
				else if (result == NFD_CANCEL) {
					puts("User pressed cancel.");
				}
				else {
					printf("Error: %s\n", NFD_GetError());
				}
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
			if (ImGui::MenuItem("Close", "Ctrl+W")) { my_tool_active = false; }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::Begin("Tree Viewer");
	DrawTree(root);
	ImGui::End();

	// Display contents in a scrolling region
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
	ImGui::BeginChild("Scrolling");
	for (int n = 0; n < 10; n++)
		ImGui::Text("Model.%04d: Some text", n);
	ImGui::EndChild();
	ImGui::End();
}

void Main_GUI_Loop(double time)
{
	double currentTime = time;
	frameCount++;

	if (currentTime - previousTimeFPS >= 1.0) // If one second has passed
	{
		fps = frameCount;
		frameCount = 0;
		previousTimeFPS = currentTime;
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// Frame Start

	ImGui::ShowMetricsWindow();

	ImGui::Text("FPS %d", fps);

	playAnimationButton();
	SceneWindow();

	LoadFile();

	// Frame End
	ImGui::Render();
}

#endif // !1
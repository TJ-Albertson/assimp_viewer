#ifndef GUI_H
#define GUI_H

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <nfd/nfd.h>
#include <scene_graph.h>
#include <camera.h>

bool animationPlaying = false;
double previousTimeFPS;
int frameCount = 0;
int fps = 0;

int nodeId = 0;

Camera* PlayerCamera;

void CollisionWindow()
{
    ImGui::Begin("Collision");

    ImGui::Text("Player Position: %f  %f  %f", playerPosition.x, playerPosition.y, playerPosition.z);

    ImGui::CollapsingHeader("Polygons");
    ImGui::BeginChild("Scrolling");
    
    for (size_t i = 0; i < potentialColliders.size(); ++i) {
        ImGui::Text("Face: %d", i);
        ImGui::Text("    Vertices: ");

        for (int j = 0; j < 4; ++j) {
            glm::vec3 vertex = potentialColliders[i].vertices[j];
            ImGui::Text("       {%.2f,%.2f,%.2f} ", vertex.x, vertex.y, vertex.z);
        }
        
         ImGui::Text("\n    Normal: %.2f %.2f %.2f\n", potentialColliders[i].normal.x, potentialColliders[i].normal.y, potentialColliders[i].normal.z);
    }
    ImGui::EndChild();

    ImGui::End();
}

void playAnimationButton()
{
    ImGui::Begin("ImGui Window");
    if (ImGui::Button("Play Animation")) {
        animationPlaying = !animationPlaying;
    }
    
    if (PlayerCamera->Type == 0) {
        ImGui::Text("Free Camera");
    } else {
        ImGui::Text("Thirdperson Camera");
    }
    
    if (ImGui::Button("Switch Camera")) {
        switchCamera(PlayerCamera);
    }
    
    ImGui::End();
}

void DrawTree(const SceneNode node)
{
    if (ImGui::TreeNode(node.name)) {
        ImGui::Text("m_NumMeshes: %d", node.model->m_NumMeshes);
        ImGui::Text("m_NumAnimations: %d", node.model->m_NumAnimations);

        for (int i = 0; i < node.numChildren; ++i)
            DrawTree(node.children[i]);
        ImGui::TreePop();
    }
}

void SceneWindow()
{
    bool my_tool_active = true;

    ImGui::Begin("Menu", &my_tool_active, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open..", "Ctrl+O")) {

                nfdchar_t* outPath = NULL;
                nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

                if (result == NFD_OKAY) {

                    puts("Success!");
                    puts(outPath);
                    std::cout << "Path: " << outPath << std::endl;


                    Model* vampire = LoadModel(outPath);
                    AddNodeToScene(0, vampire, 0);

                    free(outPath);
                } else if (result == NFD_CANCEL) {
                    puts("User pressed cancel.");
                } else {
                    printf("Error: %s\n", NFD_GetError());
                }
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { }
            if (ImGui::MenuItem("Close", "Ctrl+W")) {
                my_tool_active = false;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGui::CollapsingHeader("Scene")) {
        for (int i = 0; i < rootNode->numChildren; ++i)
            DrawTree(rootNode->children[i]);
    }

    if (ImGui::CollapsingHeader("Model")) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
        ImGui::BeginChild("Scrolling");
        for (int n = 0; n < 10; n++)
            ImGui::Text("Model.%04d: Some text", n);
        ImGui::EndChild();
    }

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

    //ImGui::ShowMetricsWindow();
    //ImGui::ShowDemoWindow();

    ImGui::Text("FPS %d", fps);

    playAnimationButton();
    SceneWindow();
    CollisionWindow();

    // Frame End
    ImGui::Render();
}

#endif // !1
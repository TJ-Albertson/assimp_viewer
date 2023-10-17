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

glm::vec3 sliderColor = glm::vec3(1.0f, 1.0f, 1.0f);
float dayNightSpeed = 0.02f;

Camera* PlayerCamera;

SceneNode* selectedNode;

void CollisionWindow()
{
    ImGui::Begin("Collision");

    ImGui::Text("Player Position: %f  %f  %f", playerPosition.x, playerPosition.y, playerPosition.z);

    ImGui::CollapsingHeader("Polygons");
    ImGui::BeginChild("Scrolling");
    
    for (size_t i = 0; i < potentialColliders.size(); ++i) {
        ImGui::Text("Face: %d", i+1);
        ImGui::Text("    Vertices: ");

        for (int j = 0; j < 3; ++j) {
            glm::vec3 vertex = potentialColliders[i].vertices[j];
            ImGui::Text("       {%.2f,%.2f,%.2f} ", vertex.x, vertex.y, vertex.z);
        }
        
         ImGui::Text("\n    Normal: %.2f %.2f %.2f\n", potentialColliders[i].normal.x, potentialColliders[i].normal.y, potentialColliders[i].normal.z);
    }
    ImGui::EndChild();

    ImGui::End();
}

void Lighting()
{
    ImGui::Begin("Lighting");

    ImGui::Text("Directional Light");

    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
    ImGui::SliderFloat("##timespeed", &dayNightSpeed, 0.0f, 1.0f, "%.2f");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::SliderFloat("##redslider", &sliderColor.x, 0.0f, 1.0f, "%.2f");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    ImGui::SliderFloat("##blueslide", &sliderColor.y, 0.0f, 1.0f, "%.2f");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    ImGui::SliderFloat("##greenslider", &sliderColor.z, 0.0f, 1.0f, "%.2f");
    ImGui::PopStyleColor();

    ImGui::End();
}

void playAnimationButton()
{
    ImGui::Begin("Camera");
    if (ImGui::Button("Play Animation")) {
        animationPlaying = !animationPlaying;
    }
    
    ImGui::SliderFloat("##fovslider", &PlayerCamera->Zoom, 50, 150, "%.f");
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


void move_children(SceneNode* node, glm::vec3 translation)
{
    SceneNode* child = node->firstChild;

    while (child != NULL) {
        move_children(child, translation);
        child = child->nextSibling;
    }
}

void gui_model() {
    ImGui::Begin("Model");

    if (selectedNode) {
        ImGui::Text("currentModel: %s", selectedNode->name);

        if (ImGui::Button("Move +X")) {
            glm::mat4 model = selectedNode->m_modelMatrix;

            model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));

            selectedNode->m_modelMatrix = model;
        }

        if (ImGui::Button("Move +Y")) {
            glm::mat4 model = selectedNode->m_modelMatrix;

            model = glm::translate(model, glm::vec3(0.0f, 15.0f, 0.0f));

            selectedNode->m_modelMatrix = model;
        }
    }
    

    ImGui::End();
}

void DrawTree(SceneNode* node)
{
    if (ImGui::TreeNode(node->name)) {


        if (node->type == "model") {
            ImGui::Text("shaderID: %d", node->shaderID);
            ImGui::Text("m_NumMeshes: %d", node->model->m_NumMeshes);
            ImGui::Text("m_NumAnimations: %d", node->model->m_NumAnimations);
        }
        
        ImGui::Text("id: %d", node->id);

        if (ImGui::Button("Click Me")) {
            selectedNode = node;
        }

        SceneNode* child = node->firstChild;
        while (child != NULL) {
            DrawTree(child);
            child = child->nextSibling;
        }

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

                    int length = strlen(outPath);

                    for (int i = 0; i < length; i++) {
                        if (outPath[i] == '\\') {
                            outPath[i] = '/'; // Replace backslash with forward slash
                        }
                    }

                    CreateNode(root_node, outPath);

                    free(outPath);
                } else if (result == NFD_CANCEL) {
                    puts("User pressed cancel.");
                } else {
                    printf("Error: %s\n", NFD_GetError());
                }
            }

            if (ImGui::MenuItem("Save", "Ctrl+S")) { 
                nfdchar_t* outPath = NULL;
                nfdresult_t result = NFD_SaveDialog(".json", NULL, &outPath);

                if (result == NFD_OKAY) {
                    // The user selected a file and clicked "Save"
                    // 'outPath' contains the selected file path
                    printf("Selected file: %s\n", outPath);
                    // You can free the 'outPath' memory when you're done with it
                    free(outPath);
                }
                else if (result == NFD_CANCEL) {
                    // The user canceled the dialog
                    printf("Dialog canceled by the user.\n");
                }
                else {
                    // Handle other error cases
                    printf("Error: %s\n", NFD_GetError());
                }
            
            }
            if (ImGui::MenuItem("Close", "Ctrl+W")) {
                my_tool_active = false;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGui::CollapsingHeader("Scene")) {

        SceneNode* child = root_node->firstChild;
        while (child != NULL) {
            DrawTree(child);
            child = child->nextSibling;
        }
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

    // Frame Start
    ImGui::NewFrame();

    //ImGui::ShowMetricsWindow();
    ImGui::ShowDemoWindow();

    ImGui::Text("FPS %d", fps);

    gui_model();
    Lighting();
    playAnimationButton();
    SceneWindow();
    CollisionWindow();

    // Frame End
    ImGui::Render();
}

#endif // !1
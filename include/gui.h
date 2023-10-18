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


bool animationModal = false;

static void ShowExampleAppSimpleOverlay(bool* p_open, int fps)
{
    static int location = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (location >= 0) {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    } else if (location == -2) {
        // Center window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Example: Simple overlay", p_open, window_flags)) {
        ImGui::Text("Simple overlay\n"
                    "(right-click to change position)");
        ImGui::Separator();
        // FPS
        ImVec4 textColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImGui::TextColored(textColor, "FPS %d", fps);
        // **
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Custom", NULL, location == -1))
                location = -1;
            if (ImGui::MenuItem("Center", NULL, location == -2))
                location = -2;
            if (ImGui::MenuItem("Top-left", NULL, location == 0))
                location = 0;
            if (ImGui::MenuItem("Top-right", NULL, location == 1))
                location = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, location == 2))
                location = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, location == 3))
                location = 3;
            if (p_open && ImGui::MenuItem("Close"))
                *p_open = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}




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
    glm::mat4 model = node->m_modelMatrix;

    model = glm::translate(model, translation);

    node->m_modelMatrix = model;

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
            move_children(selectedNode, glm::vec3(5.0f, 0.0f, 0.0f));
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


void MainMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // Handle the "Open" action
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // Handle the "Save" action
            }
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Handle the "Exit" action
                // Example: glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Animations")) {
            if (ImGui::MenuItem("Camera")) { 
                animationModal = true;
            }
            ImGui::EndMenu();
        }
        // Add more menus and items here
        ImGui::EndMainMenuBar();
    }
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

    bool t = true;

    ShowExampleAppSimpleOverlay(&t, fps);

    if (animationModal) {
        playAnimationButton();
    }


    MainMenuBar();
    gui_model();
    Lighting();
    
    SceneWindow();
    CollisionWindow();

    // Frame End
    ImGui::Render();
}

#endif // !1
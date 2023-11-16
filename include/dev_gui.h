#ifndef DEV_GUI_H
#define DEV_GUI_H

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

//#include <camera.h>
#include <input.h>
#include <nfd/nfd.h>
#include <scene_graph.h>
#include <my_math.h>

#include "gltf.h"

bool animationPlaying = false;
double previousTimeFPS;
int frameCount = 0;
int fps = 0;

int nodeId = 0;

glm::vec3 sliderColor = glm::vec3(1.0f, 1.0f, 1.0f);
float dayNightSpeed = 0.02f;

Camera* playerCamera;
SceneNode* selectedNode;

bool polygonMode = false;
bool showCollisionData = false;

float animationSpeed = 0.0f;
float animationBlend = 0.0f;

float speedModifier = 1.0f;

Material selectedMaterial;
bool showTextureWindow = true;

static void ShowExampleAppSimpleOverlay(bool* p_open, int fps)
{
    static int location = 1;
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

void CollisionData()
{
    ImGui::Begin("Collision");

    ImGui::Text("Player Position: %f  %f  %f", playerPosition.x, playerPosition.y, playerPosition.z);

    ImGui::CollapsingHeader("Polygons");
    ImGui::BeginChild("Scrolling");

    for (size_t i = 0; i < potentialColliders.size(); ++i) {
        ImGui::Text("Face: %d", i + 1);
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

void TextureWindow()
{
    ImGui::Begin("Textures");

    ImVec2 windowSize = ImGui::GetWindowSize();

    // Assuming you want the image to be square, you can use the smaller dimension as both width and height
    float imageSize = std::min(windowSize.x, windowSize.y);
    imageSize -= 50;

    if (ImGui::BeginTabBar("TextureTypes", 0)) {
        if (ImGui::BeginTabItem("BaseColor Texture")) {
            ImGui::Image((ImTextureID)selectedMaterial.m_BaseColorTextureId, ImVec2(imageSize, imageSize));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Normal Texture")) {
            ImGui::Image((void*)(intptr_t)selectedMaterial.m_NormalTextureId, ImVec2(imageSize, imageSize));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Metallic Texture")) {
            ImGui::Image((void*)(intptr_t)selectedMaterial.m_MetallicTextureId, ImVec2(imageSize, imageSize));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Roughness Texture")) {
            ImGui::Image((void*)(intptr_t)selectedMaterial.m_RoughnessTextureId, ImVec2(imageSize, imageSize));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Occlusion Texture")) {
            ImGui::Image((void*)(intptr_t)selectedMaterial.m_OcclusionTextureId, ImVec2(imageSize, imageSize));
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
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

void TransformModel()
{
    ImGui::Begin("Model");

    if (selectedNode) {
        ImGui::Text("Selected Model: %s", selectedNode->name);

        glm::mat4 modelMatrix = selectedNode->m_modelMatrix;

        glm::vec3 worldTranslation = glm::vec3(modelMatrix[3]);
        glm::vec3 worldScale = glm::vec3(modelMatrix[0][0], modelMatrix[1][1], modelMatrix[2][2]);
        glm::vec3 worldRotation;
        worldRotation.x = glm::degrees(atan2(modelMatrix[2][1], modelMatrix[2][2]));
        worldRotation.y = glm::degrees(atan2(-modelMatrix[2][0], glm::length(glm::vec2(modelMatrix[2][1], modelMatrix[2][2]))));
        worldRotation.z = glm::degrees(atan2(modelMatrix[1][0], modelMatrix[0][0]));


        ImGui::Separator();
        ImGui::SeparatorText("World Position");
        ImGui::Text("Translation: %0.2f %0.2f %0.2f", worldTranslation.x, worldTranslation.y, worldTranslation.z);
        ImGui::Text("   Rotation: %0.2f %0.2f %0.2f", worldRotation.x, worldRotation.y, worldRotation.z);
        ImGui::Text("      Scale: %0.2f %0.2f %0.2f", worldScale.x, worldScale.y, worldScale.z);


        glm::vec3 translation = selectedNode->m_pos;
        glm::vec3 rotation = selectedNode->m_eulerRot;
        glm::vec3 scale = selectedNode->m_scale;

        ImGui::Separator();
        ImGui::SeparatorText("Translation");
        ImGui::DragFloat("Translation X", &translation.x, 0.005f);
        ImGui::DragFloat("Translation Y", &translation.y, 0.005f);
        ImGui::DragFloat("Translation Z", &translation.z, 0.005f);

        ImGui::SeparatorText("Rotation");
        ImGui::DragFloat("Rotation X", &rotation.x, 0.005f);
        ImGui::DragFloat("Rotation Y", &rotation.y, 0.005f);
        ImGui::DragFloat("Rotation Z", &rotation.z, 0.005f);

        ImGui::SeparatorText("Scale");
        ImGui::DragFloat("Scale X", &scale.x, 0.005f);
        ImGui::DragFloat("Scale Y", &scale.y, 0.005f);
        ImGui::DragFloat("Scale Z", &scale.z, 0.005f);


        if (translation != selectedNode->m_pos) {
            selectedNode->m_pos = translation;
            MarkChildNodes(selectedNode);
        }

        if (scale != selectedNode->m_scale) {
            selectedNode->m_scale = scale;
            MarkChildNodes(selectedNode);
        }

        if (rotation != selectedNode->m_eulerRot) {
            selectedNode->m_eulerRot = rotation;
            MarkChildNodes(selectedNode);
        }

    } else {
        ImGui::Text("No Model Selected");
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
    ImGui::Begin("Scene");

    if (root_node != NULL) {
        SceneNode* child = root_node->firstChild;
        while (child != NULL) {
            DrawTree(child);
            child = child->nextSibling;
        }
    }
   

    ImGui::End();
}

void AABB_Tree(AABB_node* node)
{
    AABB aabb = node->aabb;

    std::string type = "NODE";

    if (node->type == LEAF) {
        type = "LEAF";
    }

    if ( ImGui::TreeNode(type.c_str()) ) {
        ImGui::Text("Minimum coordinates: (%f, %f, %f)\n", aabb.min.x, aabb.min.y, aabb.min.z);
        ImGui::Text("Maximum coordinates: (%f, %f, %f)\n", aabb.max.x, aabb.max.y, aabb.max.z);

        if (type == "NODE") {
            AABB_Tree(node->left);
            AABB_Tree(node->right);
        }

        ImGui::TreePop();
    }
}

void AABB_Window()
{
    ImGui::Begin("AABB");

    for (int i = 0; i < hitboxes.size(); i++) {

        std::string headerName = std::string("AABB %d", i);

        ImGui::CollapsingHeader("AABB");

        Hitbox hitbox = hitboxes[i];
        AABB_node* node = hitbox.rootAABB;
        AABB aabb = node->aabb;

        glm::mat4 matrix = *hitbox.m_Matrix;

        std::string matrixStr;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                matrixStr += std::to_string(matrix[i][j]);
                if (j < 3) {
                    matrixStr += ", ";
                }
            }
            if (i < 3) {
                matrixStr += "\n";
            }
        }

        // Inside your ImGui rendering loop
        ImGui::Text("Matrix Contents:");
        ImGui::Text(matrixStr.c_str());

        if (ImGui::TreeNode("Root Node")) {

            ImGui::Text("Minimum coordinates: (%f, %f, %f)\n", aabb.min.x, aabb.min.y, aabb.min.z);
            ImGui::Text("Maximum coordinates: (%f, %f, %f)\n", aabb.max.x, aabb.max.y, aabb.max.z);

            AABB_Tree(node->left);
            AABB_Tree(node->right);

            ImGui::TreePop();
        }

        

    }

    ImGui::End();
}

void MainMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
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
                } else if (result == NFD_CANCEL) {
                    // The user canceled the dialog
                    printf("Dialog canceled by the user.\n");
                } else {
                    // Handle other error cases
                    printf("Error: %s\n", NFD_GetError());
                }
            }
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Handle the "Exit" action
                // Example: glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Open")) {
            }
            if (ImGui::MenuItem("Save")) {
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Model")) {
            if (ImGui::MenuItem("Open")) {
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Camera")) {

            static int e = 0;

            ImGui::Text("Camera Type");
            ImGui::RadioButton("Free", &e, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Third", &e, 1);
            ImGui::SameLine();
            ImGui::RadioButton("First", &e, 2);

            playerCamera->Type = Camera_Type(e);

            ImGui::Separator();

            ImGui::Text("FOV");
            ImGui::SliderFloat("##fovslider", &playerCamera->FOV, 50, 150, "%.f");

            ImGui::Text("Zoom");
            ImGui::SliderFloat("##zoomslider", &playerCamera->Zoom, 10, 150, "%.f");

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Lighting")) {

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1 / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1 / 7.0f, 0.8f, 0.8f));
            ImGui::Button("Day");
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
            ImGui::Button("Night");
            ImGui::PopStyleColor(3);

            ImGui::Separator();

            ImGui::Text("Day/Time Speed");
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            ImGui::SliderFloat("##timespeed", &dayNightSpeed, 0.0f, 1.0f, "%.2f");
            ImGui::PopStyleColor();

            ImGui::Separator();

            ImGui::Text("Color");
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::SliderFloat("##redslider", &sliderColor.x, 0.0f, 1.0f, "%.2f");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::SliderFloat("##blueslide", &sliderColor.y, 0.0f, 1.0f, "%.2f");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::SliderFloat("##greenslider", &sliderColor.z, 0.0f, 1.0f, "%.2f");
            ImGui::PopStyleColor();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Collision")) {

            ImGui::Checkbox("Show Hitboxes", &drawHitboxes);
            ImGui::Separator();

            ImGui::Checkbox("glPolygonMode", &polygonMode);
            ImGui::Separator();

            if (ImGui::MenuItem(" Collision Data Window")) {
                showCollisionData = !showCollisionData;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Movement")) {

            ImGui::Checkbox("NoClip", &noClip);
            ImGui::Separator();

            ImGui::Checkbox("animationPlaying", &animationPlaying);
            ImGui::Separator();

            ImGui::Text("Animation Speed");
            ImGui::SliderFloat("##animationspeedslider", &animationSpeed, 0.0f, 0.1f, "%.03f");

            ImGui::Text("Animation Blend");
            ImGui::SliderFloat("##animationblendslider", &animationBlend, 0.0f, 1.0f, "%.03f");

            ImGui::Text("Speed Modifier");
            ImGui::SliderFloat("##speedModifierslider", &speedModifier, 0.9f, 1.1f, "%.03f");

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Texture")) {

            if (ImGui::MenuItem("Texture Window")) {
                showTextureWindow = !showTextureWindow;
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

    // ImGui::ShowMetricsWindow();
    ImGui::ShowDemoWindow();

    bool t = true;

    ShowExampleAppSimpleOverlay(&t, fps);

    if (showCollisionData) {
        CollisionData();
    }

    if (showTextureWindow) {
        TextureWindow();
    }

    MainMenuBar();
    SceneWindow();
    TransformModel();
    AABB_Window();

    // Frame End
    ImGui::Render();
}

#endif // !1
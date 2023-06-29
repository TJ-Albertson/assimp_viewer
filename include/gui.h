#ifndef GUI_H
#define GUI_H

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <nfd/nfd.h>


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

    LoadFile();

    // Frame End
    ImGui::Render();
}

#endif // !1
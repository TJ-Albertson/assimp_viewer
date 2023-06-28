#ifndef GUI_H
#define GUI_H

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

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

    ImGui::ShowMetricsWindow();

    ImGui::Text("FPS %d", fps);

    playAnimationButton();

    ImGui::Render();
}

#endif // !1
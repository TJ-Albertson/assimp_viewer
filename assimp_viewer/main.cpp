#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <camera.h>
#include <model.h>
#include <shader_m.h>
#include <animation.h>

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLFWwindow* InitializeWindow();

std::string filepath(std::string path);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, Camera* camera);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Camera* PlayerCamera;

int main()
{
    // "C:/Users/tjalb/source/repos/game/resources/objects/vampire/dancing_vampire.dae"
    // "C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/objects/vampire/dancing_vampire.dae"

    GLFWwindow* window = InitializeWindow();
    PlayerCamera = CreateCameraVector(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);

    Model* vampire = LoadModel("C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/objects/vampire/dancing_vampire.dae");
    Model* container = LoadModel("C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/models/container/container.dae");

    // unsigned int animShader = createShader("C:/Users/tjalb/source/repos/game/resources/shaders/anim_model.vs", "C:/Users/tjalb/source/repos/game/resources/shaders/anim_model.fs");
    // unsigned int modelShader = createShader("C:/Users/tjalb/source/repos/game/resources/shaders/4.2.texture.vs", "C:/Users/tjalb/source/repos/game/resources/shaders/anim_model.fs");

    unsigned int animShader = createShader("C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/shaders/anim_model.vs", "C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/shaders/anim_model.fs");
    unsigned int modelShader = createShader("C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/shaders/4.2.texture.vs", "C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/shaders/anim_model.fs");

    bool animationPlaying = false;

    while (!glfwWindowShouldClose(window)) {

        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, PlayerCamera);
       

        // imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("ImGui Window");
        if (ImGui::Button("Play Animation"))
        {
            animationPlaying = !animationPlaying;
        }
        ImGui::End();

        ImGui::Render();


        // render
        // ------
        glClearColor(1.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        // don't forget to enable shader before setting uniforms
        glUseProgram(animShader);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(PlayerCamera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = GetViewMatrix(*PlayerCamera);

        setShaderMat4(animShader, "projection", projection);
        setShaderMat4(animShader, "view", view);
        
        if (animationPlaying) {
            AnimateModel(deltaTime, vampire->m_Animations[0], vampire->rootSkeletonNode, vampire->m_FinalBoneMatrices);
        }
        

        for (int i = 0; i < 100; ++i)
            setShaderMat4(animShader, "finalBonesMatrices[" + std::to_string(i) + "]", vampire->m_FinalBoneMatrices[i]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.4f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(.5f, .5f, .5f)); // it's a bit too big for our scene, so scale it down
        setShaderMat4(animShader, "model", model);

        DrawModel(vampire, animShader);

        //std::cout << "PlayerCamera->Front.x: " << PlayerCamera->Front.x << std::endl;
        glUseProgram(modelShader);
        setShaderMat4(modelShader, "projection", projection);
        setShaderMat4(modelShader, "view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
        setShaderMat4(modelShader, "model", model);
        DrawModel(container, modelShader);




        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

GLFWwindow* InitializeWindow()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Viewer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);


    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    return window;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Camera* camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        CameraProcessKeyboard(camera, FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        CameraProcessKeyboard(camera, BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        CameraProcessKeyboard(camera, LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        CameraProcessKeyboard(camera, RIGHT, deltaTime);

}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    int rightMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    if (rightMouseButtonState == GLFW_PRESS) {
        
       glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        CameraProcessMouseMovement(PlayerCamera, xoffset, yoffset);
        
    } else if (rightMouseButtonState == GLFW_RELEASE) {
    // Enable the cursor when the right mouse button is released
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    CameraProcessMouseScroll(PlayerCamera, yoffset);
}

std::string filepath(std::string path)
{
    std::filesystem::path currentDir = std::filesystem::current_path();
    return currentDir.string().append(path);
}
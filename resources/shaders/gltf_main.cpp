#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <animation.h>
#include <collision.h>
#include <dev_gui.h>
#include <grid.h>
#include <my_math.h>
#include <shader_m.h>
#include <skybox.h>
#include <terrain.h>

#include <log_file_functions.h>
#include <scene_graph.h>

GLFWwindow* window;

unsigned int SCR_WIDTH = 2000;
unsigned int SCR_HEIGHT = 1200;
const float RENDER_DISTANCE = 1000.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

GLFWwindow* InitializeWindow()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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
    // alpha values
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ImGui initialization
    // -----------------------------

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

    SCR_HEIGHT = height;
    SCR_WIDTH = width;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    int rightMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    if (rightMouseButtonState == GLFW_PRESS) {

        mousePressed = true;

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

        ProcessMouseMovement(playerCamera, xoffset, yoffset, true);

    } else if (rightMouseButtonState == GLFW_RELEASE) {
        // Enable the cursor when the right mouse button is released
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
        mousePressed = false;
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ProcessMouseScroll(playerCamera, yoffset);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void ProcessInput(GLFWwindow* window, Camera* camera, glm::vec3& velocity, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ProcessKeyboard(camera, FORWARD, velocity, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ProcessKeyboard(camera, BACKWARD, velocity, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ProcessKeyboard(camera, LEFT, velocity, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ProcessKeyboard(camera, RIGHT, velocity, dt);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        ProcessKeyboard(camera, JUMP, velocity, dt);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        ProcessKeyboard(camera, SPRINT, velocity, dt);
}

int main()
{

    window = InitializeWindow();
    playerCamera = CreateCameraVector(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);
    /*
     * GLTF Load
     */
    //LoadGLTF("C:/Users/tjalb/OneDrive/Documents/assets/gltf/cube3.gltf");

    g_Model gltf_model;
    gltf_load_model("C:/Users/tjalb/OneDrive/Documents/assets/gltf/sphere/sphere2.gltf", gltf_model);

    //Material mat = load_gltf_material(globalMaterials[0], globalImages, globalSamplers, globalTextures);


    
    selectedMaterial = gltf_model.m_Materials[0];


    //unsigned int VAO = gltf_LoadMeshVertexData(testMesh.vertices, testMesh.indices, testMesh.numVertices, testMesh.numIndices);

    unsigned int pbrShader = createShader(filepath("/shaders/pbr/pbr.vs"), filepath("/shaders/pbr/pbr.fs"));
    //unsigned int pbrShader = createShader(filepath("/shaders/basic/basic.vs"), filepath("/shaders/basic/basic.fs"));

    unsigned int grid_VAO = LoadGrid();

    float previousTime = glfwGetTime();
    float currentTime;

    glLineWidth(2.0f);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // lights
    // ------
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 2.0f, 7.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(150.0f, 150.0f, 150.0f),
    };

    glUseProgram(pbrShader);
    setShaderInt(pbrShader, "albedoMap", 0);
    setShaderInt(pbrShader, "normalMap", 1);
    setShaderInt(pbrShader, "metallicMap", 2);
    setShaderInt(pbrShader, "roughnessMap", 3);
    setShaderInt(pbrShader, "aoMap", 4);
    

    while (!glfwWindowShouldClose(window)) {

        currentTime = glfwGetTime();
        float deltaTime = previousTime - currentTime;

        UpdateCameraVectors(playerCamera, playerState.position);

        float radius = 10;
        float timeScale = 0.5;

        float x = sin(glfwGetTime() * timeScale) * radius;
        float y = sin(glfwGetTime() * timeScale) * 2;
        float z = cos(glfwGetTime() * timeScale) * radius;

        lightPositions[0] = glm::vec3(x, y, z);

        Main_GUI_Loop(currentTime);

        // render
        // ------
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(playerCamera->FOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, RENDER_DISTANCE);
        glm::mat4 view = GetViewMatrix(*playerCamera);

        glUseProgram(pbrShader);
        setShaderMat4(pbrShader, "projection", projection);
        setShaderMat4(pbrShader, "view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        setShaderMat3(pbrShader, "normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        setShaderMat4(pbrShader, "model", model);

        gltf_draw_mesh(gltf_model.m_Meshes[0].m_VAO, gltf_model.m_Materials[0], gltf_model.m_Meshes[0].m_NumIndices, pbrShader);
        // draw_gltf_mesh(VAO, mat, testMesh.numIndices, pbrShader);

        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            setShaderVec3(pbrShader, "lightPositions[" + std::to_string(i) + "]", newPos);
            setShaderVec3(pbrShader, "lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            setShaderMat4(pbrShader, "model", model);
            setShaderMat3(pbrShader, "normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            gltf_draw_mesh(gltf_model.m_Meshes[0].m_VAO, gltf_model.m_Materials[0], gltf_model.m_Meshes[0].m_NumIndices, pbrShader);
        }

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
    return 1;
}
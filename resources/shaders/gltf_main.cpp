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

#include "gltf_model.h"

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
    LoadGLTF("C:/Users/tjalb/OneDrive/Documents/assets/gltf/untitled.gltf");

     Material mat = load_gltf_material(globalMaterials[0], globalImages, globalSamplers, globalTextures);

    for (int k = 0; k < testMesh.numIndices; ++k) {

        printf("    total_indices[%d]: %u\n", k, testMesh.indices[k]);
    }

    printf("\nvertexes\n");
    for (int k = 0; k < testMesh.numVertices; ++k) {

        glm::vec3 position = testMesh.vertices[k].m_Position;
        glm::vec3 normal = testMesh.vertices[k].m_Normal;
        glm::vec2 texcoord = testMesh.vertices[k].m_TexCoord_0;

        printf("    vertex[%d]\n", k);
        printf("        position: %f %f %f\n", position.x, position.y, position.z);
        printf("          normal: %f %f %f\n", normal.x, normal.y, normal.z);
        printf("        texCoord: %f %f \n", texcoord.x, texcoord.y);
    }

    unsigned int VAO = gltf_LoadMeshVertexData(testMesh.vertices, testMesh.indices, testMesh.numVertices, testMesh.numIndices);

    unsigned int pbrShader = createShader(filepath("/shaders/pbr/pbr.vs"), filepath("/shaders/pbr/pbr.fs"));
    //unsigned int pbrShader = createShader(filepath("/shaders/basic/basic.vs"), filepath("/shaders/basic/basic.fs"));

    unsigned int grid_VAO = LoadGrid();

    float previousTime = glfwGetTime();
    float currentTime;


    glLineWidth(2.0f);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    printf("drawMesh.numIndices: %d\n", drawMesh.numIndices);
    while (!glfwWindowShouldClose(window))
    {

        currentTime = glfwGetTime();
        float deltaTime = previousTime - currentTime;

        UpdateCameraVectors(playerCamera, playerState.position);

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
        setShaderMat4(pbrShader, "model", model);


        // draw_gltf_mesh(drawMesh.VAO, mat, drawMesh.numIndices, pbrShader);
        draw_gltf_mesh(VAO, mat, testMesh.numIndices, pbrShader);


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
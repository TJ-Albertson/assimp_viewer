#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION

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

#include <model.h>
#include <shader_m.h>
#include <animation.h>
#include <grid.h>
#include <gui.h>
#include <skybox.h>
#include <collision.h>

#include <scene_graph.h>
#include <log_file_functions.h>

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const float RENDER_DISTANCE = 1000.0f;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// timing
float deltaTime = 0.0f;
float previousTime;

GLFWwindow* InitializeWindow();

std::string filepath(std::string path);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, Camera* camera);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


float acceleration = 0.15f;
float max_speed = 10.0f;
float friction = 2.0f;

bool jumpDebounce = false;

bool isJumping = false;
float jumpDuration = 0.7f;

glm::vec3 CubicInterpolate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    float a = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float b = -2.0f * t3 + 3.0f * t2;
    float c = t3 - 2.0f * t2 + t;
    float d = t3 - t2;

    return a * p1 + b * p2 + c * (p2 - p0) + d * (p3 - p1);
}



glm::vec3 cubicInterpolate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    glm::vec3 a0 = p2 - p1 - p0 + p1;
    glm::vec3 a1 = p0 - p1 - a0;
    glm::vec3 a2 = p2 - p0;
    glm::vec3 a3 = p1;

    return a0 * t3 + a1 * t2 + a2 * t + a3;
}

glm::vec3 cubicSplineInterpolate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    glm::vec3 a0 = p3 - p2 - p0 + p1;
    glm::vec3 a1 = p0 - p1 - a0;
    glm::vec3 a2 = p2 - p0;
    glm::vec3 a3 = p1;

    return a0 * t3 + a1 * t2 + a2 * t + a3;
}

glm::vec3 cubicInterpolate2(
    glm::vec3 x0, glm::vec3 x1,
    glm::vec3 x2, glm::vec3 x3,
    float t)
{
    glm::vec3 a = (3.0f * x1 - 3.0f * x2 + x3 - x0) / 2.0f;
    glm::vec3 b = (2.0f * x0 - 5.0f * x1 + 4.0f * x2 - x3) / 2.0f;
    glm::vec3 c = (x2 - x0) / 2.0f;
    glm::vec3 d = x1;

    return a * t * t * t + b * t * t + c * t + d;
}

glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t)
{
    return x * (1.f - t) + y * t;
}


int main()
{
    GLFWwindow* window = InitializeWindow();
    PlayerCamera = CreateCameraVector(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);


    unsigned int grid_VAO = LoadGrid();

    //unsigned int modelShader  = createShader(filepath("/shaders/4.2.texture.vs"), filepath("/shaders/anim_model.fs"));
    unsigned int modelShader = createShader(filepath("/shaders/6.multiple_lights.vs"), filepath("/shaders/6.multiple_lights.fs"));
    shaderIdArray[0] = modelShader;
    unsigned int animShader   = createShader(filepath("/shaders/anim_model.vs"),  filepath("/shaders/anim_model.fs"));
    unsigned int gridShader   = createShader(filepath("/shaders/grid.vs"),        filepath("/shaders/grid.fs"));
    unsigned int hitboxShader = createShader(filepath("/shaders/4.2.texture.vs"), filepath("/shaders/hitbox.fs"));
    //unsigned int lightShader  = createShader(filepath("/shaders/6.multiple_lights.vs"), filepath("/shaders/6.multiple_lights.fs"));

    unsigned int billboardShader = createShader(filepath("/shaders/billboard.vs"), filepath("/shaders/billboard.fs"));

    Model* billboard = LoadModel(filepath("/resources/models/billboards/hp1.obj"));
    Model* moon = LoadModel(filepath("/resources/models/billboards/moon.obj"));
    Model* sun = LoadModel(filepath("/resources/models/billboards/sun.obj"));


    Model* player = LoadModel(filepath("/resources/objects/vampire/dancing_vampire.dae"));

    Model* sphere = LoadModel(filepath("/resources/models/sphere/sphere.obj"));

    Model* soid_man = LoadModel(filepath("/resources/models/man/soid_man.obj"));


    Model* arrow = LoadModel(filepath("/resources/models/direction_arrows/z.obj"));


    if (LoadScene(filepath("/resources/scenes/scene1.json"))) {
        printf("LoadScene Failed!\n");
    }


    LoadSkybox(filepath);

    // Wireframe mode
    // --------------------
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glm::vec3 color = glm::vec3(0.0f);

    const double debounceDelay = 1.5; // 200 milliseconds
    double lastSpacePressTime = 0.0;

    bool isSpaceKeyPressed = false;
    float maxJumpDuration = 2.5f;  // Adjust this to control the jump duration (in seconds)
    float jumpStartTime = 0.0f;

     glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f)
    };
    
    while (!glfwWindowShouldClose(window)) {

        // per-frame time logic
        // --------------------
        float currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        // input
        // -----
        processInput(window, PlayerCamera);
      
        //imgui
        Main_GUI_Loop(currentTime);

        // render
        // ------
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(PlayerCamera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, RENDER_DISTANCE);
        glm::mat4 view = GetViewMatrix(*PlayerCamera);


        /*
        // Draw Grid, with instance array
        glUseProgram(gridShader);
        setShaderMat4(gridShader, "projection", projection);
        setShaderMat4(gridShader, "view", view);
        glBindVertexArray(grid_VAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
        glBindVertexArray(0);
        */

       
        glUseProgram(animShader);
        
        // (shader, uniformName, value)
        setShaderMat4(animShader, "projection", projection);
        setShaderMat4(animShader, "view", view);
        
        if (animationPlaying) {
            AnimateModel(deltaTime, player->m_Animations[0], player->rootSkeletonNode, player->m_FinalBoneMatrices);
        }
        
        for (int i = 0; i < 100; ++i)
            setShaderMat4(animShader, "finalBonesMatrices[" + std::to_string(i) + "]", player->m_FinalBoneMatrices[i]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
        model = glm::scale(model, glm::vec3(.5f, .5f, .5f));
        setShaderMat4(animShader, "model", model);

        //DrawModel(vampire, animShader);

        glUseProgram(modelShader);

        setVec3(modelShader, "viewPos", PlayerCamera->Position);
        setFloat(modelShader, "material.shininess", 32.0f);


        float radius = 1.0f; // You can adjust the radius of the circle
        float angular_speed = 0.02f; // You can adjust the speed of rotatiom
        // Calculate the x, y, and z coordinates of the vector
        float x = radius * cos(angular_speed * currentTime);
        float y = radius * sin(angular_speed * currentTime);
        float z = 0.0f; // Since you want it to move along the y and z axes

        glm::vec3 sunDirection = -glm::vec3(x, y, z);

        float red_value, green_value, blue_value;
        red_value = green_value = blue_value = sunDirection.y;

      
        float sun_t = sunDirection.y;
        //printf("sunDirection: %f %f %f\n", sunDirection.x, sunDirection.y, sunDirection.z);

        glm::vec3 orange = glm::vec3(1., 0.741, 0.086);
        glm::vec3 purple = glm::vec3(0.082f, 0.0f, 0.298f);
        glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);

        sun_t *= 2.5f;
        if (sun_t > 0 && sun_t < 1) {
            //color = cubicInterpolate2(white, orange, white, white, sun_t);
            color = lerp(orange, white, sun_t);
        }
        if (sun_t < 0 && sun_t > -1) {
            //color = cubicInterpolate(glm::vec3(1.0f, 1.0f, 0.0f), orange, purple,glm::abs(sun_t));
            color = lerp(orange, purple, glm::abs(sun_t));
            //color = cubicInterpolate2(purple, orange, purple, purple, glm::abs(sun_t));
        }

        


        // i have y value that bounces between -1 and 1; day = 0 < x < 1; night = -1 < x < 0; 

        //printf("color: %f %f %f\n", sunDirection.y, green_value, sunDirection.y);
        

        setVec3(modelShader, "dirLight.direction", sunDirection);
        setVec3(modelShader, "dirLight.ambient", color.x, color.y, color.z);
        setVec3(modelShader, "dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        setVec3(modelShader, "dirLight.specular", 0.5f, 0.5f, 0.5f);

        // point light 1
        setVec3(modelShader, "pointLights[0].position", playerPosition);
        setVec3(modelShader, "pointLights[0].ambient", 1.0f, 1.0f, 1.0f);
        setVec3(modelShader, "pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        setVec3(modelShader, "pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        setFloat(modelShader, "pointLights[0].constant", 1.0f);
        setFloat(modelShader, "pointLights[0].linear", 0.09f);
        setFloat(modelShader, "pointLights[0].quadratic", 0.032f);

        // point light 2
        setVec3(modelShader, "pointLights[1].position", pointLightPositions[1]);
        setVec3(modelShader, "pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        setVec3(modelShader, "pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        setVec3(modelShader, "pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        setFloat(modelShader, "pointLights[1].constant", 1.0f);
        setFloat(modelShader, "pointLights[1].linear", 0.09f);
        setFloat(modelShader, "pointLights[1].quadratic", 0.032f);

        // point light 3
        setVec3(modelShader, "pointLights[2].position", pointLightPositions[2]);
        setVec3(modelShader, "pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        setVec3(modelShader, "pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        setVec3(modelShader, "pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        setFloat(modelShader, "pointLights[2].constant", 1.0f);
        setFloat(modelShader, "pointLights[2].linear", 0.09f);
        setFloat(modelShader, "pointLights[2].quadratic", 0.032f);

        // point light 4
        setVec3(modelShader, "pointLights[3].position", pointLightPositions[3]);
        setVec3(modelShader, "pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        setVec3(modelShader, "pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        setVec3(modelShader, "pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        setFloat(modelShader, "pointLights[3].constant", 1.0f);
        setFloat(modelShader, "pointLights[3].linear", 0.09f);
        setFloat(modelShader, "pointLights[3].quadratic", 0.032f);

        // spotLight
        setVec3(modelShader, "spotLight.position", PlayerCamera->Position);
        setVec3(modelShader, "spotLight.direction", PlayerCamera->Front);
        setVec3(modelShader, "spotLight.ambient", 0.0f, 0.0f, 0.0f);
        setVec3(modelShader, "spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        setVec3(modelShader, "spotLight.specular", 1.0f, 1.0f, 1.0f);
        setFloat(modelShader, "spotLight.constant", 1.0f);
        setFloat(modelShader, "spotLight.linear", 0.09f);
        setFloat(modelShader, "spotLight.quadratic", 0.032f);
        setFloat(modelShader, "spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        setFloat(modelShader, "spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        setShaderMat4(modelShader, "projection", projection);
        setShaderMat4(modelShader, "view", view);


        DrawScene(root_node);


        glUseProgram(billboardShader);

        setShaderMat4(billboardShader, "projection", projection);
        setShaderMat4(billboardShader, "view", view);

        glm::vec3 tmp = (sunDirection * 100.0f);
        //tmp.y += 100.0f;

       // printf("sunDirection: %f %f %f\n", tmp.x, tmp.y, tmp.z);

        glm::mat4 b_model = glm::mat4(1.0f);
        b_model = glm::translate(b_model, tmp);
        setShaderMat4(billboardShader, "model", b_model);

        DrawModel(sun, billboardShader);

        b_model = glm::mat4(1.0f);
        b_model = glm::translate(b_model, -tmp);
        setShaderMat4(billboardShader, "model", b_model);

        DrawModel(moon, billboardShader);

        glUseProgram(modelShader);
        

        // Player
        model = glm::mat4(1.0f);
        model = glm::translate(model, playerPosition);

        glm::vec3 vector;
        
        if (isMoving) {
            playerVelocity += acceleration;

            if (playerVelocity > max_speed)
                playerVelocity = max_speed;

        } else {
            playerVelocity -= friction;
            if (playerVelocity < 0) playerVelocity = 0.0f;
        }

        

        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            if (currentTime - lastSpacePressTime >= debounceDelay) {
                movePlayer(glm::vec3(0.0f, -1.0f, 0.0f));
                lastSpacePressTime = currentTime;
            }
        }


        //need 2 fix dis           ?switch to adding new velocity to old? idk
        vector = (glm::normalize(directionVector) * playerVelocity) * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (currentTime - jumpStartTime >= debounceDelay) {
                jumpStartTime = currentTime;
                isJumping = true;
                vector += glm::vec3(0.0f, 0.5f, 0.0f); // Apply initial y-velocity
            }
        }
        if (isJumping) {
            // Keep applying y-velocity within the jump duration
            vector += glm::vec3(0.0f, 0.5f - (currentTime - jumpStartTime), 0.0f);
            
            if (currentTime - jumpStartTime >= jumpDuration) {
                isJumping = false; // Stop jumping after jumpDuration
               // Reset y-velocity when the jump duration is over
            }
        }


        movePlayer(vector);


        if (PlayerCamera->Type == THIRDPERSON) {
           model = glm::rotate(model, playerRotation, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        //model = glm::scale(model, glm::vec3(.025f, .025f, .025f));
        setShaderMat4(modelShader, "model", model);
        DrawModel(soid_man, modelShader);


        glm::vec3 playerCenter = playerPosition; //+glm::vec3(0.0f, 2.6f, 0.0f);
        glm::vec3 sourcePoint = playerCenter;

        // Player direction arrows
        glm::mat4 arrowModelMatrix = glm::mat4(1.0f); // Identity matrix
        arrowModelMatrix = glm::translate(arrowModelMatrix, playerCenter);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); // Define the up vector
        glm::mat4 rotationMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(vector.x, vector.y, -vector.z), upVector);

       
        //arrowModelMatrix = glm::rotate(arrowModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        arrowModelMatrix *= rotationMatrix;
        setShaderMat4(modelShader, "model", arrowModelMatrix);
        DrawModel(arrow, modelShader);


        // BACKFACE CULLING |ON|
        glEnable(GL_CULL_FACE);

        glUseProgram(hitboxShader);

        setShaderMat4(hitboxShader, "projection", projection);
        setShaderMat4(hitboxShader, "view", view);

        // Player hitbox
        model = glm::mat4(1.0f);
        model = glm::translate(model, playerCenter);
        setShaderMat4(hitboxShader, "model", model);
        setShaderVec4(hitboxShader, "color", glm::vec4(1.0f, 0.0f, 0.0f, 0.3f));
        DrawModel(sphere, hitboxShader);

        // BACKFACE CULLING |OFF|
        glDisable(GL_CULL_FACE);

        // Needs to be drawn last; covers up everything after it
        DrawSkybox(*PlayerCamera, view, projection);   
 

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

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        CameraProcessKeyboard(camera, FORWARD_RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        CameraProcessKeyboard(camera, FORWARD_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        CameraProcessKeyboard(camera, BACKWARD_RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        CameraProcessKeyboard(camera, BACKWARD_LEFT, deltaTime);

	if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) && PlayerCamera->Type == THIRDPERSON && mousePressed) {
		isMoving = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !mousePressed) {
		isMoving = true;
    }
    else {
        isMoving = false;
    }

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

        CameraProcessMouseMovement(PlayerCamera, xoffset, yoffset);
        
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
    CameraProcessMouseScroll(PlayerCamera, yoffset);
}


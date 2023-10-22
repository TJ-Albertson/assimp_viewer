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
#include <dev_gui.h>
#include <skybox.h>
#include <collision.h>
#include <my_math.h>
#include <terrain.h>

#include <scene_graph.h>
#include <log_file_functions.h>

// settings
unsigned int SCR_WIDTH = 2000;
unsigned int SCR_HEIGHT = 1200;
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

int main()
{
    GLFWwindow* window = InitializeWindow();
    PlayerCamera = CreateCameraVector(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);
    
    unsigned int grid_VAO = LoadGrid();

    unsigned int basicShader  = createShader(filepath("/shaders/basic/basic.vs"), filepath("/shaders/basic/basic.fs"));
    unsigned int modelShader = createShader(filepath("/shaders/6.multiple_lights.vs"), filepath("/shaders/6.multiple_lights.fs"));
    shaderIdArray[0] = modelShader;
    unsigned int animShader   = createShader(filepath("/shaders/anim_model.vs"),  filepath("/shaders/anim_model.fs"));
    unsigned int gridShader   = createShader(filepath("/shaders/grid.vs"),        filepath("/shaders/grid.fs"));
    unsigned int hitboxShader = createShader(filepath("/shaders/hitbox.vs"), filepath("/shaders/hitbox.fs"));
    shaderIdArray[1] = hitboxShader;
    unsigned int animatedShader = createShader(filepath("/shaders/animated_texture.vs"), filepath("/shaders/animated_texture.fs"));
    //unsigned int lightShader  = createShader(filepath("/shaders/6.multiple_lights.vs"), filepath("/shaders/6.multiple_lights.fs"));

    unsigned int billboardShader = createShader(filepath("/shaders/billboard.vs"), filepath("/shaders/billboard.fs"));

    Model* billboard = LoadModel(filepath("/resources/models/billboards/hp1.obj"));
    Model* moon = LoadModel(filepath("/resources/models/billboards/moon.obj"));
    Model* sun = LoadModel(filepath("/resources/models/billboards/sun.obj"));


    Model* player = LoadModel(filepath("/resources/objects/vampire/dancing_vampire.dae"));

    Model* sphere = LoadModel(filepath("/resources/models/sphere/sphere.obj"));

    Model* soid_man = LoadModel(filepath("/resources/models/man/soid_man.obj"));

    Model* arrow = LoadModel(filepath("/resources/models/direction_arrows/z.obj"));

    //Model* cube = LoadModel(filepath("/resources/models/cube/cube_outline.obj"));
    //unsigned int cube = CreateHitbox();

    if (LoadScene(filepath("/resources/scenes/scene2.json"))) {
        printf("LoadScene Failed!\n");
    }

    LoadSkybox(filepath, "skybox7");
    //LoadTerrain(filepath, filepath("/resources/textures/heightmaps/map1.png"));
    


    
    // Wireframe mode
    // --------------------
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glLineWidth(2.0f);

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
        glUseProgram(basicShader);
        setShaderMat4(basicShader, "projection", projection);
        setShaderMat4(basicShader, "view", view);
       
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
        setShaderFloat(modelShader, "material.shininess", 32.0f);
        //setInt(modelShader, "material.diffuse", 0);
        //setInt(modelShader, "material.specular", 1);
        //setInt(modelShader, "material.emission", 2);



        float radius = 1.0f; // You can adjust the radius of the circle
        float angular_speed = dayNightSpeed; // You can adjust the speed of rotatiom
        // Calculate the x, y, and z coordinates of the vector

        float time = currentTime + 160.0f;
        float x = radius * cos(angular_speed * time);
        float y = radius * sin(angular_speed * time);
        float z = 0.0f; // Since you want it to move along the y and z axes

        glm::vec3 sunDirection = -glm::vec3(x, y, z);
        float sun_t = sunDirection.y;

        glm::vec3 orange = glm::vec3(1.0f, 0.741f, 0.086f);
        glm::vec3 purple = glm::vec3(0.082f, 0.0f, 0.298f);
        glm::vec3 white  = glm::vec3(0.9f, 1.0f, 0.9f);

        sun_t *= 2.5f;
        if (sun_t > 0 && sun_t < 1) {
            color = lerp(orange, white, sun_t);
        }
        if (sun_t < 0 && sun_t > -1) {
            color = lerp(orange, purple, glm::abs(sun_t));
        }
        

        setVec3(modelShader, "dirLight.direction", glm::vec3(-0.5f, -1.0f, 0.0f));
        setVec3(modelShader, "dirLight.ambient", sliderColor.x, sliderColor.y, sliderColor.z);
        setVec3(modelShader, "dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        setVec3(modelShader, "dirLight.specular", 0.5f, 0.5f, 0.5f);

        // point light 1
        setVec3(modelShader, "pointLights[0].position", playerPosition);
        setVec3(modelShader, "pointLights[0].ambient", 1.0f, 1.0f, 1.0f);
        setVec3(modelShader, "pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        setVec3(modelShader, "pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        setShaderFloat(modelShader, "pointLights[0].constant", 1.0f);
        setShaderFloat(modelShader, "pointLights[0].linear", 0.09f);
        setShaderFloat(modelShader, "pointLights[0].quadratic", 0.06f);

        // point light 2
        setVec3(modelShader, "pointLights[1].position", pointLightPositions[1]);
        setVec3(modelShader, "pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        setVec3(modelShader, "pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        setVec3(modelShader, "pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        setShaderFloat(modelShader, "pointLights[1].constant", 1.0f);
        setShaderFloat(modelShader, "pointLights[1].linear", 0.09f);
        setShaderFloat(modelShader, "pointLights[1].quadratic", 0.09f);

        // point light 3
        setVec3(modelShader, "pointLights[2].position", pointLightPositions[2]);
        setVec3(modelShader, "pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        setVec3(modelShader, "pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        setVec3(modelShader, "pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        setShaderFloat(modelShader, "pointLights[2].constant", 1.0f);
        setShaderFloat(modelShader, "pointLights[2].linear", 0.09f);
        setShaderFloat(modelShader, "pointLights[2].quadratic", 0.032f);

        // point light 4
        setVec3(modelShader, "pointLights[3].position", pointLightPositions[3]);
        setVec3(modelShader, "pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        setVec3(modelShader, "pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        setVec3(modelShader, "pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        setShaderFloat(modelShader, "pointLights[3].constant", 1.0f);
        setShaderFloat(modelShader, "pointLights[3].linear", 0.09f);
        setShaderFloat(modelShader, "pointLights[3].quadratic", 0.032f);

        setShaderMat4(modelShader, "projection", projection);
        setShaderMat4(modelShader, "view", view);

        








        glLineWidth(1.0f);
        glUseProgram(hitboxShader);
        setShaderMat4(hitboxShader, "projection", projection);
        setShaderMat4(hitboxShader, "view", view);
        //DrawTerrain(view, projection, sunDirection, color, PlayerCamera->Position);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawScene(root_node);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);







        /*
        AABB_node aabb1_node = updateAABB(hitboxes[0].rootAABB, (*hitboxes[0].m_Matrix));
        AABB_node aabb2_node = updateAABB(hitboxes[1].rootAABB, (*hitboxes[1].m_Matrix));

        AABB aabb1 = aabb1_node.aabb;
        AABB aabb2 = aabb2_node.aabb;
        */
        
        /*
        printf("aabb1\n");
        printf("Minimum coordinates: (%f, %f, %f)\n", aabb1.min.x, aabb1.min.y, aabb1.min.z);
        printf("Maximum coordinates: (%f, %f, %f)\n", aabb1.max.x, aabb1.max.y, aabb1.max.z);

        printf("aabb2\n");
        printf("Minimum coordinates: (%f, %f, %f)\n", aabb2.min.x, aabb2.min.y, aabb2.min.z);
        printf("Maximum coordinates: (%f, %f, %f)\n", aabb2.max.x, aabb2.max.y, aabb2.max.z);
        */

        //BVHCollision(&aabb1_node, &aabb2_node);

        AABB_node aabb1_node = *hitboxes[0].rootAABB;
        AABB_node aabb2_node = *hitboxes[1].rootAABB;

        ///AABB_node a = aabb1_node;
        //AABB_node b = aabb2_node;

        AABB_AABB_Collision(*hitboxes[0].rootAABB, *hitboxes[1].rootAABB, hitboxes[0].m_Matrix, hitboxes[1].m_Matrix);




        



        glUseProgram(animatedShader);
        setShaderMat4(animatedShader, "projection", projection);
        setShaderMat4(animatedShader, "view", view);

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
        //DrawModel(soid_man, modelShader);


        glm::vec3 playerCenter = playerPosition; //+glm::vec3(0.0f, 2.6f, 0.0f);
        glm::vec3 sourcePoint = playerCenter;

         

        // BACKFACE CULLING |ON|
        glEnable(GL_CULL_FACE);

        glUseProgram(hitboxShader);
        setShaderMat4(hitboxShader, "projection", projection);
        setShaderMat4(hitboxShader, "view", view);



        // Collision Point Ball
        model = glm::mat4(1.0f);
        model = glm::translate(model, playerCenter + glm::normalize(vectorPosition));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        setShaderMat4(hitboxShader, "model", model);
        setShaderVec4(hitboxShader, "color", glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
        //DrawModel(sphere, hitboxShader);

        // Collision Point Ball
        model = glm::mat4(1.0f);
        model = glm::translate(model, collisionBallPosition);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        setShaderMat4(hitboxShader, "model", model);
        setShaderVec4(hitboxShader, "color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        //DrawModel(sphere, hitboxShader);

        // Player hitbox
        model = glm::mat4(1.0f);
        model = glm::translate(model, playerCenter);
        setShaderMat4(hitboxShader, "model", model);
        setShaderVec4(hitboxShader, "color", glm::vec4(1.0f, 0.0f, 0.0f, 0.3f));
        //DrawModel(sphere, hitboxShader);    




        // BACKFACE CULLING |OFF|
        glDisable(GL_CULL_FACE);
        
        
        setShaderVec4(hitboxShader, "color", glm::vec4(1.0f, 0.0f, 0.5f, 0.5f));
        setShaderMat4(hitboxShader, "model", glm::mat4(1.0f));

        
        

        // Needs to be drawn last; covers up everything after it
        DrawSkybox(*PlayerCamera, view, projection, currentTime);   
        //update, except for transparent stuff i guess

        
        
        /*

        float scale = 10.0f;
        float x_size = 3;
        float y_size = 1;
        float z_size = 3;
         
        for (int x = -x_size; x <= x_size; x++) {
           for (int y = -y_size; y <= y_size; y++) {
                for (int z = -z_size; z <= z_size; z++) {
                    glm::mat4 cube_space = glm::mat4(1.0f);
                    cube_space = glm::translate(cube_space, glm::vec3(x, y, z) * scale);
                    cube_space = glm::scale(cube_space, glm::vec3(scale));
                    setShaderMat4(hitboxShader, "model", cube_space);
                    setShaderVec4(hitboxShader, "color", glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));
                    DrawHitbox(cube, hitboxShader);
                }
           }
        }
        */

        glUseProgram(billboardShader);
        setShaderMat4(billboardShader, "projection", projection);
        setShaderMat4(billboardShader, "view", view);

        glm::vec3 sunPosition = (sunDirection * 500.0f);

        glm::mat4 b_model = glm::mat4(1.0f);
        b_model = glm::translate(b_model, sunPosition);
        setShaderMat4(billboardShader, "model", b_model);

        DrawModel(sun, billboardShader);

        b_model = glm::mat4(1.0f);
        b_model = glm::translate(b_model, -sunPosition);
        setShaderMat4(billboardShader, "model", b_model);

        DrawModel(moon, billboardShader);

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
	} else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && PlayerCamera->Type == THIRDPERSON && !mousePressed) {
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
    //CameraProcessMouseScroll(PlayerCamera, yoffset);
}


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

//#include <utils.h>

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

int main()
{
    GLFWwindow* window = InitializeWindow();
    PlayerCamera = CreateCameraVector(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), YAW, PITCH);

    InitSceneNode();

    unsigned int grid_VAO = LoadGrid();

 
   
    unsigned int animShader  = createShader(filepath("/shaders/anim_model.vs"),  filepath("/shaders/anim_model.fs"));
    unsigned int modelShader = createShader(filepath("/shaders/4.2.texture.vs"), filepath("/shaders/anim_model.fs"));
    unsigned int gridShader  = createShader(filepath("/shaders/grid.vs"), filepath("/shaders/grid.fs"));

    unsigned int hitboxShader = createShader(filepath("/shaders/4.2.texture.vs"), filepath("/shaders/hitbox.fs"));


    Model* player = LoadModel(filepath("/resources/objects/vampire/dancing_vampire.dae"));
    AddNodeToScene(0, player, animShader);

    Model* container = LoadModel(filepath("/resources/models/container/container.dae"));
    AddNodeToScene(0, container, modelShader);

    Model* green_alpha = LoadModel(filepath("/resources/models/green_alpha/green_alpha.obj"));
    AddNodeToScene(0, green_alpha, modelShader);

    Model* sphere = LoadModel(filepath("/resources/models/sphere/sphere.obj"));
    AddNodeToScene(0, sphere, modelShader);

    Model* labeled_alpha_cube = LoadModel(filepath("/resources/models/labeled_alpha_cube/single_tri.obj"));
    AddNodeToScene(0, labeled_alpha_cube, modelShader);

    Model* single_tri = LoadModel(filepath("/resources/models/planes/plane.obj"));
    AddNodeToScene(0, single_tri, modelShader);





    Model* x_arrow = LoadModel(filepath("/resources/models/direction_arrows/x.obj"));
    AddNodeToScene(0, x_arrow, modelShader);
    Model* y_arrow = LoadModel(filepath("/resources/models/direction_arrows/y.obj"));
    AddNodeToScene(0, y_arrow, modelShader);
    Model* z_arrow = LoadModel(filepath("/resources/models/direction_arrows/z.obj"));
    AddNodeToScene(0, z_arrow, modelShader);


    glm::mat4 hitbox = glm::mat4(1.0f);
    hitbox = glm::translate(hitbox, glm::vec3(5.0f, 1.0f, 5.0f));
    hitbox = glm::scale(hitbox, glm::vec3(1.0f, 1.0f, 1.0f));

    create_hitbox(filepath("/resources/models/labeled_alpha_cube/single_tri.obj"), glm::vec3(5.0f, 1.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    glm::mat4 hitbox2 = glm::mat4(1.0f);
    hitbox2 = glm::translate(hitbox2, glm::vec3(-5.0f, -0.5f, 5.0f));
    hitbox2 = glm::scale(hitbox2, glm::vec3(1.0f, 1.0f, 1.0f));
    //create_hitbox(filepath("/resources/models/labeled_alpha_cube/labeled_alpha_tri.obj"), glm::vec3(-5.0f, -0.5f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f));


    glm::mat4 single_tri_mat = glm::mat4(1.0f);
    single_tri_mat = glm::translate(single_tri_mat, glm::vec3(10.0f, 0.0f, 10.0f));
    single_tri_mat = glm::scale(single_tri_mat, glm::vec3(50.0f, 1.0f, 50.0f));
    //(filepath("/resources/models/planes/plane.obj"), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(50.0f, 1.0f, 50.0f));

    //Model* skybox = LoadModel(filepath("/resources/objects/skybox/skybox.obj"));
    //AddNodeToScene(0, skybox, modelShader);

    // Model* grass = LoadModel(filepath("/resources/objects/grass_cube/grass_cube.obj"));
    //AddNodeToScene(0, grass, modelShader);

    //Model* grass_plane = LoadModel(filepath("/resources/objects/grass_plane/grass_plane.obj"));
    // AddNodeToScene(0, grass_plane, modelShader);

    PrintSceneHierarchy(rootNode);

    LoadSkybox(filepath);

    //Model* backpack = LoadModel(filepath("resources/objects/cyborg/cyborg.obj"));
   // AddNodeToScene(0, backpack, modelShader);

    // Wireframe mode
    // --------------------
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    const double debounceDelay = 0.5; // 200 milliseconds

    // Define a variable to keep track of the last time the space key was pressed
    double lastSpacePressTime = 0.0;


    glm::vec4 hitboxColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);
    
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

        // draw grid with instance array
        glUseProgram(gridShader);
        setShaderMat4(gridShader, "projection", projection);
        setShaderMat4(gridShader, "view", view);
        glBindVertexArray(grid_VAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
        glBindVertexArray(0);

        
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
        setShaderMat4(modelShader, "projection", projection);
        setShaderMat4(modelShader, "view", view);




        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, 0.0f, 10.0f));
        model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
        setShaderMat4(modelShader, "model", model);
        DrawModel(container, modelShader);



       


        


        // PLayer
        model = glm::mat4(1.0f);
        model = glm::translate(model, playerPosition);

        glm::vec3 vector;
        
        vector = glm::normalize(directionVector) * playerVelocity * deltaTime;
        if (playerPosition.y > 0.0f) {
            
        }

        if (isMoving) {
            playerVelocity += acceleration;

            if (playerVelocity > max_speed) playerVelocity = max_speed;

            
            
            movePlayer(vector);

        } else {
            playerVelocity -= friction;
            if (playerVelocity < 0) playerVelocity = 0.0f;
            //playerVelocity = 0;
            //directionVector = glm::vec3(0.001f, 0.001f, 0.001f);
        }
        //need 2 fix dis           ?switch to adding new velocity to old? idk
         
        

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (currentTime - lastSpacePressTime >= debounceDelay) {
                movePlayer(glm::vec3(0.0f, 1.0f, 0.0f));
                lastSpacePressTime = currentTime;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            if (currentTime - lastSpacePressTime >= debounceDelay) {
                movePlayer(glm::vec3(0.0f, -1.0f, 0.0f));
                lastSpacePressTime = currentTime;
            }
        }


        if (PlayerCamera->Type == THIRDPERSON) {
           model = glm::rotate(model, playerRotation, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        model = glm::scale(model, glm::vec3(.025f, .025f, .025f));
        setShaderMat4(modelShader, "model", model);
        //DrawModel(player, modelShader);



        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, 10.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        setShaderMat4(modelShader, "model", model);
        //DrawModel(grass, modelShader);


        model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(10.0f, 10.0f, 0.0f));
        model = glm::scale(model, glm::vec3(20.0f, 0.0f, 20.0f));
        setShaderMat4(modelShader, "model", model);
        // DrawModel(grass_plane, modelShader);

        //DrawScene();




        model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(5.0f, 3.0f, 5.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        setShaderMat4(modelShader, "model", model);
        DrawModel(x_arrow, modelShader);
        DrawModel(y_arrow, modelShader);
        DrawModel(z_arrow, modelShader);



         // player hitbox
        glm::vec3 playerCenter = playerPosition; //+glm::vec3(0.0f, 2.6f, 0.0f);
        glm::vec3 sourcePoint = playerCenter;

        glm::mat4 arrowModelMatrix = glm::mat4(1.0f); // Identity matrix
        arrowModelMatrix = glm::translate(arrowModelMatrix, playerCenter);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); // Define the up vector
        glm::mat4 rotationMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(vector.x, vector.y, -vector.z), upVector);

       
        //arrowModelMatrix = glm::rotate(arrowModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        arrowModelMatrix *= rotationMatrix;
        setShaderMat4(modelShader, "model", arrowModelMatrix);
        DrawModel(z_arrow, modelShader);


        glEnable(GL_CULL_FACE);

        //hitboxes

        setShaderMat4(modelShader, "model", hitbox);
        DrawModel(labeled_alpha_cube, modelShader);

        setShaderMat4(modelShader, "model", hitbox2);
        //DrawModel(labeled_alpha_cube, modelShader);
        
        setShaderMat4(modelShader, "model", single_tri_mat);
        //DrawModel(single_tri, modelShader);

        glUseProgram(hitboxShader);

        setShaderMat4(hitboxShader, "projection", projection);
        setShaderMat4(hitboxShader, "view", view);

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
           hitboxColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);
        }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
           hitboxColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.3f);
        }

       


        


        model = glm::mat4(1.0f);
        model = glm::translate(model, playerCenter);

         // playercenter
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        setShaderMat4(hitboxShader, "model", model);
        setShaderVec4(hitboxShader, "color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        DrawModel(sphere, hitboxShader);

        // nearestpointpolygon
        model = glm::mat4(1.0f);
        model = glm::translate(model, collisionBallPosition);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        setShaderVec4(hitboxShader, "color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        setShaderMat4(hitboxShader, "model", model);
        DrawModel(sphere, hitboxShader);


         // sphereIntersectionPointPos
        model = glm::mat4(1.0f);
        model = glm::translate(model, sphereIntersectionPointPos);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        setShaderVec4(hitboxShader, "color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        setShaderMat4(hitboxShader, "model", model);
        DrawModel(sphere, hitboxShader);


        // sphere hitbox
        // 2.5
        model = glm::mat4(1.0f);
        model = glm::translate(model, playerCenter);
        setShaderMat4(hitboxShader, "model", model);
        setShaderVec4(hitboxShader, "color", hitboxColor);
        DrawModel(sphere, hitboxShader);

       

        

        setShaderMat4(hitboxShader, "model", hitbox);
        setShaderVec4(hitboxShader, "color", hitboxColor);
        //DrawModel(green_alpha, hitboxShader);


    

        glDisable(GL_CULL_FACE);

        // needs to be drawn last; covers up everything after it
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
    } else {
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

std::string filepath(std::string path)
{
    std::filesystem::path currentDir = std::filesystem::current_path();

    std::string projectPath = currentDir.string();

    for (char& c : projectPath) {
        if (c == '\\') {
            c = '/';
        }
    }

    std::string toRemove = "/assimp_viewer";

    size_t pos = projectPath.rfind(toRemove);
    if (pos != std::string::npos) {
        projectPath.erase(pos, toRemove.length());
    }
 
    return projectPath.append(path);
}
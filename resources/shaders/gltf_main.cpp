#include <glad/glad.h>
#include <GLFW/glfw3.h>


#define STB_IMAGE_IMPLEMENTATION

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
#include <model.h>
#include <my_math.h>
#include <shader_m.h>
#include <skybox.h>
#include <terrain.h>
#include <gltf.h>

#include <log_file_functions.h>
#include <scene_graph.h>

int main()
{
    /*
     * GLTF Load
     */
    LoadGLTF("C:/Users/tjalb/OneDrive/Documents/assets/gltf/cube.gltf");

    return 1;
}
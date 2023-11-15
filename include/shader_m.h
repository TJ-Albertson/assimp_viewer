#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void checkCompileErrors(GLuint shader, std::string type);


// constructor generates the shader on the fly
// ------------------------------------------------------------------------
unsigned int createShader(std::string vertexPathStr, std::string fragmentPathStr)
{
    unsigned int shaderID;
    const char* vertexPath = vertexPathStr.c_str();
    const char* fragmentPath = fragmentPathStr.c_str();

    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);


    try {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // close file handlers
        vShaderFile.close();
        fShaderFile.close();

        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. compile shaders
    unsigned int vertex, fragment;

    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // shader Program
    shaderID = glCreateProgram();
    glAttachShader(shaderID, vertex);
    glAttachShader(shaderID, fragment);
    glLinkProgram(shaderID);
    checkCompileErrors(shaderID, "PROGRAM");

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shaderID;
}



// utility uniform functions
// ------------------------------------------------------------------------
void setShaderBool(unsigned int shaderID, const std::string& name, bool value)
{
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void setShaderInt(unsigned int shaderID, const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void setShaderFloat(unsigned int shaderID, const std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void setShaderVec2(unsigned int shaderID, const std::string& name, const glm::vec2& value)
{
    glUniform2fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}
void setShaderVec2(unsigned int shaderID, const std::string& name, float x, float y)
{
    glUniform2f(glGetUniformLocation(shaderID, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void setShaderVec3(unsigned int shaderID, const std::string& name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}
void setVec3(unsigned int shaderID, const std::string& name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(shaderID, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void setShaderVec4(unsigned int shaderID, const std::string& name, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}
void setVec4(unsigned int shaderID, const std::string& name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(shaderID, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void setMat2(unsigned int shaderID, const std::string& name, const glm::mat2& mat)
{
    glUniformMatrix2fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void setShaderMat3(unsigned int shaderID, const std::string& name, const glm::mat3& mat)
{
    glUniformMatrix3fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void setShaderMat4(unsigned int shaderID, const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}


// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

#endif
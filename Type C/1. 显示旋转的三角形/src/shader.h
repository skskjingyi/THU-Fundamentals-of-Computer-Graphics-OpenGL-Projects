#ifndef SHADER_H
#define SHADER

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

class Shader
{
public:
    GLuint Program;

    // shader constructor
    Shader(const GLchar* vertexPath, const char* fragmentPath)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            //open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;

            // read the entire shader file into stream
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            // close file handlers
            vShaderFile.close();
            fShaderFile.close();

            // convert the shader files into strings
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();

        }
        catch(std::ifstream::failure e)
        {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << '\n';
        }
        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar* fShadercode = fragmentCode.c_str();

        unsigned int vertexShader;
        unsigned int fragmentShader;
        int success;
        char infoLog[512];

        // vertex shader
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout <<"ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        
        //fragment shader
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShadercode, NULL);
        glCompileShader(fragmentShader);
        // check for compile error
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout <<"ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        //shader program linking vertex and fragment
        // create shader program to link vertex and fragment
        this->Program = glCreateProgram(); //returns ID reference to the program object
        glAttachShader(this->Program, vertexShader);
        glAttachShader(this->Program, fragmentShader);
        glLinkProgram(this->Program);
        // check for linking error
        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
            std::cout <<"ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        // delete the shaders after linking into the program object
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    
    ~Shader() {glDeleteProgram(this->Program);}

    // use the current shader
    void Use() { glUseProgram(this->Program);}
};

#endif
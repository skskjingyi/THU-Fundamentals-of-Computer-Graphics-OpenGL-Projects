#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

// functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

GLFWwindow* initializeWindow();

// settings
const int WIDTH = 800;
const int HEIGHT = 600;
const char* WINDOW_NAME = "Spinning Triangle";
glm::vec3 triangleColor(0.1f, 0.3f, 0.9f); //default color: blue

int main()
{   
    GLFWwindow* window = initializeWindow();
    if (!window)
    {
        return 0;
    }

    Shader ourShader("resources/main.vert", "resources/main.frag");

    // after the shaders, we set up vertex data and configure vertex attributes
    // vertices for a triangle, 3D cordinate xyz range between -1.0 and 1.0
    float vertices[] = 
    {
        // Positions        
        0.5f,  -0.5f, 0.0f,  // Bottom Right
       -0.5f,  -0.5f, 0.0f,  // Bottom Left
        0.0f,  0.5f,  0.0f,  // Top
    };

    // object
    unsigned int VBO, VAO;
    // send to vertex shader
    // store the vertices on vertex buffer objects (VBO) in GPU's memory
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // 1. bind vertex array object
    glBindVertexArray(VAO);
    // 2. binding buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // copy vertex data into buffer's memory
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. link vertex data to vertex shader attribute
    // vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); //unbind vao

    // render loop
    while(!glfwWindowShouldClose(window))
    {
        
        // input
        processInput(window);

        // rendering
        glClearColor(0.8f, 0.7f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.Use();
        GLint uColorLoc = glGetUniformLocation(ourShader.Program, "triangleColor");
        glUniform3fv(uColorLoc, 1, &triangleColor[0]);

        // Create transformations
        glm::mat4 model(1);
        glm::mat4 view(1);
        glm::mat4 projection(1);
        model = glm::rotate(model, -(GLfloat)glfwGetTime() * glm::radians(50.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        
        projection = glm::perspective(
        glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
         // Get their uniform locations
         
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));        
        
        // draw triangle
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);


        glfwSwapBuffers(window);
        glfwPollEvents();  
    }
    
    // de-allocate all resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // terminate and delete resources
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

//define functions
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        triangleColor = glm::vec3(0.9f, 0.2f, 0.1f); // Red
    }
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        triangleColor = glm::vec3(0.2f, 0.9f, 0.1f); // Green
    }
    if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        triangleColor = glm::vec3(0.1f, 0.3f, 0.9f); // Blue
    }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow* initializeWindow()
{
    // initialize GLFW, version 
    int glfwInitRes = glfwInit();
    if (!glfwInitRes)
    {
        std::cerr << "Unable to initalize GLFW" << std::endl;
        return nullptr;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_NAME, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    // set current OpenGL context
    glfwMakeContextCurrent(window);

    // set the required callback function
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // load OpenGL function pointers with GLAD
    int gladInitRes = gladLoadGL();
    if (!gladInitRes)
    {
        std::cout << "unable to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    return window;

}
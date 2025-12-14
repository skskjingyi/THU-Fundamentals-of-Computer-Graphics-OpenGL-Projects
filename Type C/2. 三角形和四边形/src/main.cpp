#include <GLFW/glfw3.h>
#include <glad/glad.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "shader.h"

using std::cerr;
using std::endl;

// Function prototypes
void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mode);

// Settings
const int WIDTH = 800;
const int HEIGHT = 600;

GLFWwindow* initialize() {
  // Init GLFW
  int glfwInitRes = glfwInit();
  if (!glfwInitRes) {
    cerr << "Unable to initialize GLFW" << endl;
    return nullptr;
  }

  // Set all the required options for GLFW
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);  // Disable resizing

  // Create a GLFWwindow object that we can use for GLFW's functions
  GLFWwindow* window =
      glfwCreateWindow(WIDTH, HEIGHT, "Moving Triangle & Square", nullptr, nullptr);
  if (!window) {
    cerr << "Unable to create GLFW window" << endl;
    glfwTerminate();
    return nullptr;
  }

  // Set current OpenGL context
  glfwMakeContextCurrent(window);

  // Set the required callback functions
  glfwSetKeyCallback(window, keyCallback);

  // Load OpenGL function pointers with GLAD
  int gladInitRes = gladLoadGL();
  if (!gladInitRes) {
    cerr << "Unable to initialize GLAD" << endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return nullptr;
  }

  // Define the viewport dimensions
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // Setup OpenGL options
  glEnable(GL_DEPTH_TEST);

  return window;
}

// The MAIN function, from here we start the application and run the game loop
int main() {
  GLFWwindow* window = initialize();
  if (!window) {
    return 0;
  }

  // Build and compile our shader program
  Shader ourShader("resources/shaders/main.vert",
                   "resources/shaders/main.frag");

  // Set up vertex data (and buffer(s)) and attribute pointers
  GLfloat vertices[] = {
       // Positions           // color
        -1.0f,  -0.8f, 0.0f,  0.6f, 0.8f, 1.0f,// Bottom Right
        -0.5f,  -0.5f, 0.0f,  0.6f, 1.0f, 0.8f,// Bottom Left
        -0.8f,  0.5f,  0.0f,  1.0f, 0.8f, 0.5f,// Top};

        // square             // color
        0.4f,  0.0f, 0.0f,    0.8f, 0.5f, 1.0f,
        0.4f,  0.4f, 0.0f,    1.0f, 0.5f, 0.8f,
        0.8f,  0.4f,  0.0f,   1.0f, 0.5f, 0.5f,
        0.4f, 0.0f, 0.0f,     0.8f, 0.5f, 1.0f,
        0.8f, 0.4f, 0.0f,     1.0f, 0.5f, 0.5f,
        0.8f, 0.0f, 0.0f,      0.6f, 0.8f, 1.0f

      };  

  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  // Bind the Vertex Array Object first, then bind and set vertex buffer(s),
  // and then configure vertex attributes(s).
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                        (GLvoid*)0);
  glEnableVertexAttribArray(0);
  
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);  // Unbind VAO
  


  // Game loop
  while (!glfwWindowShouldClose(window)) {
    // Check if any events have been activiated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();

    // Render
    // Clear the colorbuffer
    glClearColor(1.0f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate shader
    ourShader.Use();
    GLint shadingModeLoc = glGetUniformLocation(ourShader.Program, "shadingMode");
    // Create transformations
    float time = glfwGetTime();

    glm::mat4 view(1);
    glm::mat4 projection(1);

    // Note: currently we set the projection matrix each frame, but since the
    // projection matrix rarely changes it's often best practice to set it
    // outside the main loop only once.
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
    
    // Get their uniform locations
    GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
    GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
    GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");

    // Pass them to the shaders
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Draw container
    glBindVertexArray(VAO);


    // ============Drawing Triangle=======
    glm::vec3 center(-0.77f, -0.27f, 0.0f); //center for left triangle
    glm::mat4 model1(1.0f);
    
    // translation trajectory left and right
    glm::vec3 path1(0.2 * sin(time), 0.0f, 0.0f);

    model1 = glm::translate(model1, path1); 
    model1 = glm::translate(model1, center);   
    model1 = glm::scale(model1, glm::vec3(0.8f, 0.8f, 1.0f));  
    model1 = glm::rotate(model1, time * -glm::radians(50.0f), glm::vec3(0.0f, 0.0f, 1.0f));  // spin on z axis
    model1 = glm::translate(model1, -center);    
    
    // first triangle
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
    glUniform1i(shadingModeLoc, 1);   // 1 = flat
    glDrawArrays(GL_TRIANGLES, 0, 3);  // 36 vertices: 2 triangles per face * 6

    // ==========draw square==========
    glm::mat4 model2(1);
    glm::vec3 center2 = {0.6f, 0.2f, 0.0f};
    glm::vec3 path2(0.0f, 0.3f*sin(time * 0.7f), 0.0f);   //moving up and down

    model2 = glm::translate(model2, path2);
    model2 = glm::translate(model2, center2);
    model2 = glm::rotate(model2, time * glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model2 = glm::translate(model2, -center2);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
    glUniform1i(shadingModeLoc, 0);   
    glDrawArrays(GL_TRIANGLES, 3, 6);


    glBindVertexArray(0);

    // Swap the screen buffers
    glfwSwapBuffers(window);
  }

  // Properly de-allocate all resources once they've outlived their purpose
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  // Terminate GLFW, clearing any resources allocated by GLFW.
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mode) {
                    
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

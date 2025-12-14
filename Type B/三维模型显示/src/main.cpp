#include <GLFW/glfw3.h>
#include <glad/glad.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "shader.h"
#include "hedge.h"

using std::cerr;
using std::endl;

// =================Function prototypes=================
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

// =================Window Settings=====================
const int WIDTH = 800;
const int HEIGHT = 600;   

// Mouse-controlled camera rotation
bool   gMousePressed = false;
double gLastMouseX   = 0.0;
double gLastMouseY   = 0.0;
float  gYaw          = 0.0f;  // rotate around Y
float  gPitch        = 0.0f;  // rotate around X

// Mode control keys: 1 vertext only (default), 2 face only, 3 edges only, 4 face+edge
int drawMode = 1;
// ================== Helper Functions ==================

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
      glfwCreateWindow(WIDTH, HEIGHT, "Object Loader", nullptr, nullptr);
  if (!window) {
    cerr << "Unable to create GLFW window" << endl;
    glfwTerminate();
    return nullptr;
  }

  // Set current OpenGL context
  glfwMakeContextCurrent(window);

  // Set the required callback functions
  glfwSetKeyCallback(window, keyCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetCursorPosCallback(window, cursorPosCallback);

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

  // transparency image
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  return window;
}

// The MAIN function, from here we start the application and run the game loop
int main() {
  GLFWwindow* window = initialize();
  if (!window) {
    return 0;
  }

  std::string objPath = "resources/obj/eight.uniform.obj";
  Hedge mesh;
  if (!mesh.loadFromOBJ(objPath))
  {
    std::cout << "Failed to load object: " << objPath << std::endl;
  }

  // For OpenGL buffers:
  std::vector<glm::vec3> positions;
  std::vector<unsigned int> faceIndices;
  std::vector<unsigned int> edgeIndices;

  mesh.buildVertexArray(positions);
  mesh.buildFaceIndexArray(faceIndices);
  mesh.buildEdgeIndexArray(edgeIndices);

  // one VAO for position, two EBO: one for faces one for edges
  GLuint VAO, VBO, EBOFaces, EBOEdges;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBOFaces);
  glGenBuffers(1, &EBOEdges);
  
  //vao
  glBindVertexArray(VAO);

  // vbo
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);

  // vertex attribute 0 = vec3 position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, // layout(location = 0)
    3, // vec3
    GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);  

  // ebo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOFaces);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices.size() * sizeof(unsigned int), faceIndices.data(), GL_STATIC_DRAW);  

  glBindVertexArray(0);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOEdges);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(unsigned int), edgeIndices.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);
  
  // shaders
  Shader ourShader("resources/shaders/main.vert", "resources/shaders/main.frag");

  // Game loop
  while (!glfwWindowShouldClose(window)) {
    // Check if any events have been activiated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();

    // Render
    // Clear the colorbuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader.Use();
    GLint texLoc = glGetUniformLocation(ourShader.Program, "uTexture");
    glUniform1i(texLoc, 0);  // tell shader: sampler uses texture unit 0

    // camera matrixes
    glm::mat4 model(1);
    glm::mat4 view(1);
    glm::mat4 projection(1);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));   
    view = glm::rotate(view, glm::radians(gPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(gYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

    // Get their uniform locations
    GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
    GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
    GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc,  1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc,  1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(VAO);

    GLint colLoc = glGetUniformLocation(ourShader.Program, "uColor");
    switch(drawMode)
    {
      case 1: // vertex only
        glUniform3f(colLoc, 0.7f, 0.2f, 0.4f); 
        glPointSize(4.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(positions.size()));
        
        break;
      
      case 2: //face only
        glUniform3f(colLoc, 0.5f, 0.2f, 0.8f); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOFaces);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(faceIndices.size()), GL_UNSIGNED_INT, (void*)0);
        break;
      
      case 3: //edges
        // wireframe edges
        glUniform3f(colLoc, 1.0f, 1.0f, 1.0f); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOEdges);
        glDrawElements(GL_LINES, static_cast<GLsizei>(edgeIndices.size()), GL_UNSIGNED_INT, (void*)0);
        break;
      case 4: 
        // face + edge
        // draw face
        glUniform3f(colLoc, 0.5f, 0.2f, 0.8f);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOFaces);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(faceIndices.size()), GL_UNSIGNED_INT, (void*)0);
        //draw edge
        glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOEdges);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_LINES, static_cast<GLsizei>(edgeIndices.size()), GL_UNSIGNED_INT, (void*)0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    }
    glBindVertexArray(0);
    // Swap the screen buffers
    glfwSwapBuffers(window);
  }

  // Terminate GLFW, clearing any resources allocated by GLFW.
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (key == GLFW_KEY_1 && action == GLFW_PRESS)
  {
    drawMode = 1;
  }
  if (key == GLFW_KEY_2 && action == GLFW_PRESS)
  {
    drawMode = 2;
  }
  if (key == GLFW_KEY_3 && action == GLFW_PRESS)
  {
    drawMode = 3;
  }
  if (key == GLFW_KEY_4 && action == GLFW_PRESS)
  {
    drawMode = 4;
  }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT)
  {
    if (action == GLFW_PRESS)
    {
      gMousePressed = true;
      glfwGetCursorPos(window, &gLastMouseX, &gLastMouseY);
    } else if (action == GLFW_RELEASE)
    {
      gMousePressed = false;
    }
  }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (!gMousePressed) return;

    double dx = xpos - gLastMouseX;
    double dy = ypos - gLastMouseY;
    gLastMouseX = xpos;
    gLastMouseY = ypos;

    float sensitivity = 0.2f;

    gYaw   += (float)dx * sensitivity;   // left-right drag
    gPitch += (float)dy * sensitivity;   // up-down drag

    // clamp pitch so you don't flip over
    if (gPitch > 89.0f)  gPitch = 89.0f;
    if (gPitch < -89.0f) gPitch = -89.0f;
}
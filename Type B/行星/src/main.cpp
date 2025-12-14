#include <GLFW/glfw3.h>
#include <glad/glad.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "planet.h"
#include "shader.h"
#include "sphere.h"

// stb image for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::cerr;
using std::endl;

// =================Function prototypes=================
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

// =================Window Settings=====================
const int WIDTH = 800;
const int HEIGHT = 600;
   
float gTranslateZ = 15.0f;     

// Mouse-controlled camera rotation
bool   gMousePressed = false;
double gLastMouseX   = 0.0;
double gLastMouseY   = 0.0;
float  gYaw          = 0.0f;  // rotate around Y
float  gPitch        = 0.0f;  // rotate around X

// ================== Helper Functions ==================

GLuint loadTexture(const char* paths)
{
  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);

  unsigned char* data = stbi_load(paths, &width, &height, &nrChannels, 0);
  if (data) {
      GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

      glGenerateMipmap(GL_TEXTURE_2D);
  } else {
      std::cout << "Failed to load texture: " << paths << std::endl;
  }

  stbi_image_free(data);

  return texID;
}

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
      glfwCreateWindow(WIDTH, HEIGHT, "Planets", nullptr, nullptr);
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

  PlanetSystem ps;
  ps.init();

  double lastTime = glfwGetTime();

  //======================Background Picture===================================
  // Set up vertex data (and buffer(s)) and attribute pointers                 
  float fullscreen[] = {
    // positions    // texCoords
    -1.0f, -1.0f,   0.0f, 0.0f,
     1.0f, -1.0f,   1.0f, 0.0f,
     1.0f,  1.0f,   1.0f, 1.0f,

    -1.0f, -1.0f,   0.0f, 0.0f,
     1.0f,  1.0f,   1.0f, 1.0f,
    -1.0f,  1.0f,   0.0f, 1.0f
  };
  GLuint bgVBO, bgVAO;
  glGenVertexArrays(1, &bgVAO);
  glGenBuffers(1, &bgVBO);

  glBindVertexArray(bgVAO);
  glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreen), fullscreen, GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // texture coordinate atrribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);  // Unbind VAO
  
  // ================== create sphere and name==============================
  GLuint sphereVBO, sphereVAO, sphereEBO;
  int sphereIndexCount =0;
  createSphere(1.0f, 32, 64, sphereVAO, sphereVBO, sphereEBO, sphereIndexCount);

  

  // ------------------- LOAD TEXTURES/SHADERS -------------------
  const char* bgPath = "resources/tex/background.png";
  
  GLuint bgTexture = loadTexture(bgPath);

  // Build and compile our shader program
  Shader planetShader("resources/shaders/main.vert", "resources/shaders/main.frag");
  Shader bgShader("resources/shaders/bg.vert", "resources/shaders/bg.frag");

  bgShader.Use();
  glUniform1i(glGetUniformLocation(bgShader.Program, "bgTexture"), 0);

  planetShader.Use();
  glUniform1i(glGetUniformLocation(planetShader.Program, "uTexture"), 0);
  
  lastTime = (float)glfwGetTime();
  // Game loop
  while (!glfwWindowShouldClose(window)) {
    // Check if any events have been activiated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();

    // ====== 时间更新 ======
    float currentTime = (float)glfwGetTime();
    float deltaTime   = currentTime - lastTime;
    lastTime = currentTime;

    // Render
    // Clear the colorbuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // create the background
    bgShader.Use();
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(bgVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    planetShader.Use();
    
    // camera matrixes
    glm::mat4 view(1);
    glm::mat4 projection(1);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -gTranslateZ));   
    view = glm::rotate(view, glm::radians(gPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(gYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

    
    double deltaDays = deltaTime * 5;   // 1 sec = 5 days
    ps.update(deltaDays);

    // draw
    ps.draw(view, projection, sphereVAO, sphereIndexCount, planetShader.Program); 

    // Swap the screen buffers
    glfwSwapBuffers(window);
  }

  // Properly de-allocate all resources once they've outlived their purpose
  glDeleteVertexArrays(1, &sphereVAO);
  glDeleteBuffers(1, &sphereVBO);

  // Terminate GLFW, clearing any resources allocated by GLFW.
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      switch (key) {
      case GLFW_KEY_S:
          gTranslateZ += 0.2f;  // zoom in
          break;
      case GLFW_KEY_W:
          gTranslateZ -= 0.2f;  // zoom out
          break;
      default:
          break;
      }
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
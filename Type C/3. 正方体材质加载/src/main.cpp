#include <GLFW/glfw3.h>
#include <glad/glad.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "shader.h"

// stb image for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::cerr;
using std::endl;

// Function prototypes
void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mode);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

// Settings
const int WIDTH = 800;
const int HEIGHT = 600;

bool gMousePressed = false;
bool gFirstMouse = true;
float gCubeYaw   = 0.0f;   // rotate view around Y (left-right)
float gCubePitch = 0.0f;   // rotate view around X (up-down)
double gLastX = 0.0, gLastY = 0.0;

// variable for textures
GLuint gTextures[6];
int gFilterMode = 2;  // 1=nearest, 2=linear, 3=mipmap (default linear)

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
      glfwCreateWindow(WIDTH, HEIGHT, "Textured Cube", nullptr, nullptr);
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
  glfwSetCursorPosCallback(window, mouseMoveCallback);
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
    // xyz (position coordinates), s,t(texture coordinates)
    // front face (z = -0.5)
      -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, //bottom left
      0.5f,  -0.5f, -0.5f,    1.0f, 0.0f, //bottom right
      0.5f,  0.5f,  -0.5f,    1.0f, 1.0f, //top right
      0.5f,  0.5f,  -0.5f,    1.0f, 1.0f, //top right
      -0.5f, 0.5f,  -0.5f,    0.0f, 1.0f, //top left
      -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, //bottom left

    // back face z=0.5
      -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, //bottom left
      0.5f,  -0.5f, 0.5f,    1.0f, 0.0f, //bottom right
      0.5f,  0.5f,  0.5f,    1.0f, 1.0f, //top right
      0.5f,  0.5f,  0.5f,    1.0f, 1.0f, //top right
      -0.5f, 0.5f,  0.5f,    0.0f, 1.0f, //top left
      -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, //bottom left

      // --- Left face (x = -0.5) ---
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, // top-left
    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, // top-right
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, // bottom-right

    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, // bottom-right
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, // bottom-left
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, // top-left

     // --- Right face (x = +0.5) ---
     0.5f,  0.5f,  0.5f,   0.0f, 1.0f, // top-left
     0.5f,  0.5f, -0.5f,   1.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f, // bottom-right

     0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   0.0f, 1.0f,

    // --- Bottom face (y = -0.5) ---
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   1.0f, 1.0f,

     0.5f, -0.5f,  0.5f,   1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,

    // --- Top face (y = +0.5) ---
    -0.5f,  0.5f, -0.5f,   0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   1.0f, 1.0f,

     0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 0.0f
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                        (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // texture coordinate atrribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);  // Unbind VAO

    // ------------------- LOAD 6 TEXTURES -------------------
  glGenTextures(6, gTextures);

  const char* paths[6] = {
      "tex/right.png",
      "tex/left.png",
      "tex/top.png",
      "tex/bottom.png",
      "tex/front.png",
      "tex/back.png"
  };

  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);

  for (int i = 0; i < 6; ++i) {
      glBindTexture(GL_TEXTURE_2D, gTextures[i]);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      unsigned char* data = stbi_load(paths[i], &width, &height, &nrChannels, 0);
      if (data) {
          GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

          glTexImage2D(GL_TEXTURE_2D, 0, format,
                      width, height, 0,
                      format, GL_UNSIGNED_BYTE, data);

          glGenerateMipmap(GL_TEXTURE_2D);
      } else {
          std::cout << "Failed to load texture: " << paths[i] << std::endl;
      }
      stbi_image_free(data);
  }
  // Game loop
  while (!glfwWindowShouldClose(window)) {
    // Check if any events have been activiated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();
    // Render
    // Clear the colorbuffer
    glClearColor(0.83f, 0.85f, 0.96f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate shader
    ourShader.Use();
    GLint texLoc = glGetUniformLocation(ourShader.Program, "uTexture");
    glUniform1i(texLoc, 0);  // tell shader: sampler uses texture unit 0

    // Create transformations
    glm::mat4 model(1);
    glm::mat4 view(1);
    glm::mat4 projection(1);

    // uncomment this and comment the next two model rotate line to do auto rotation
    //model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

    // using mouse to rotate the cube
    model = glm::rotate(model, glm::radians(gCubePitch), glm::vec3(1,0,0));
    model = glm::rotate(model, glm::radians(gCubeYaw), glm::vec3(0,1,0));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));   

    // Note: currently we set the projection matrix each frame, but since the
    // projection matrix rarely changes it's often best practice to set it
    // outside the main loop only once.
    projection = glm::perspective(
        glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

    // Get their uniform locations
    GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
    GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
    GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");

    // Pass them to the shaders
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw container
    glBindVertexArray(VAO);
    // Each face is 6 vertices
    for (int i = 0; i < 6; ++i) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gTextures[i]);
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
    }
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

void ApplyFilterMode()
{
    for (int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_2D, gTextures[i]);

        if (gFilterMode == 1) {
            // 最近点采样
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else if (gFilterMode == 2) {
            // 线性采样
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else if (gFilterMode == 3) {
            // mipmap + 线性
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_MIPMAP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_MIPMAP);
        }
    }
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (action == GLFW_PRESS) {
      if (key == GLFW_KEY_1) 
      {
        gFilterMode = 1; 
        ApplyFilterMode();
      }// nearest
      else if (key == GLFW_KEY_2){
        gFilterMode = 2;
        ApplyFilterMode();
       } // linear
      else if (key == GLFW_KEY_3) {
        gFilterMode = 3; 
        ApplyFilterMode();
      }// mipmap
  }

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT)
  {
    if (action == GLFW_PRESS)
    {
      gMousePressed = true;
      gFirstMouse = true;
    } else if (action == GLFW_RELEASE)
    {
      gMousePressed = false;
    }
  }
}


void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
  if (!gMousePressed) return;

  if (gFirstMouse)
  {
    gLastX = xpos;
    gLastY = ypos;
    gFirstMouse = false;
    return;
  }

  double xoffset = xpos - gLastX;
  double yoffset = gLastY - ypos;

  gLastX = xpos;
  gLastY = ypos;

  float sensitivity = 0.2f;
  gCubeYaw += static_cast<float>(xoffset) * sensitivity;
  gCubePitch += static_cast<float>(yoffset) * sensitivity;

  if (gCubePitch > 89.0f)  gCubePitch = 89.0f;
  if (gCubePitch < -89.0f) gCubePitch = -89.0f;
}
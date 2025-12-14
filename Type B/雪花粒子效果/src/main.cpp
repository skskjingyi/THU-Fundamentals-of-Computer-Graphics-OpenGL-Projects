#include <GLFW/glfw3.h>
#include <glad/glad.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "shader.h"

// stb image for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::cerr;
using std::endl;

// =================Function prototypes=================
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

// =================Window Settings=====================
const int WIDTH = 800;
const int HEIGHT = 600;

int gFilterMode = 2;  // 1=nearest, 2=linear, 3=mipmap (default linear)

// ================== Particle System ==================
struct Particle {
    glm::vec3 position;  // 雪花当前坐标
    glm::vec3 velocity;  // 雪花当前速度
    float size;          // 雪花大小（缩放用）
    bool active;         // 是否已经开始存在于场景中
};

const int MAX_PARTICLES = 1000;
std::vector<Particle> particles(MAX_PARTICLES);

int activeCount = 0;         // 当前激活的粒子数
float spawnRate = 50.0f;     // 每秒新增 N 个粒子
float elapsedTime = 0.0f;    // 程序运行时间

// ================== Helper Functions ==================
float randomFloat(float min, float max)
{
  float t = rand() / (float)RAND_MAX; // Normalize to [0,1)
  return min + (max - min) * t;
}

void respawnParticle(Particle& p)
{
  // generate in upper screen
  float x = randomFloat(-10.0f, 10.0f);
  float y = randomFloat(5.0f, 10.0f);
  float z = randomFloat(-5.0f, 5.0f);  //for 3d scene

  p.position = glm::vec3(x, y, z);

  // velocity, -y
  float vx = randomFloat(-0.5f, 0.5f);
  float vy = randomFloat(-2.0f, -1.0f);
  float vz = 0.0f;
  
  p.velocity = glm::vec3(vx, vy, vz);

  // random size of snowflake
  p.size = randomFloat(0.1f, 0.3f);

  p.active = true;
}

void updateParticles(float dt)
{
  elapsedTime += dt;

  int targetActive = (int)glm::min(elapsedTime * spawnRate, (float)MAX_PARTICLES);

  while(activeCount < targetActive)
  {
    respawnParticle(particles[activeCount]);
    activeCount++;
  }

  for (int i = 0; i < activeCount; i++)
  {
    Particle& p = particles[i];
    if (!p.active) continue;

    p.position += p.velocity * dt;

    float wind = 0.5f * sin(elapsedTime + i*0.1f);
    p.position.x += wind * dt;

    if (p.position.y < -5.0f)
    {
      respawnParticle(p);
    }
  }
}

GLuint loadTexture(const char* paths)
{
  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
      glfwCreateWindow(WIDTH, HEIGHT, "Snowing", nullptr, nullptr);
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
  Shader ourShader("resources/shaders/main.vert", "resources/shaders/main.frag");
  Shader bgShader("resources/shaders/bg.vert", "resources/shaders/bg.frag");

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
  
  // ===================snowflake==============================
  GLfloat vertices[] = {
    // xyz (position coordinates), s,t(texture coordinates)
    // square for loading snowflake
      -0.5f, -0.5f, 0.0f,    0.0f, 0.0f, //bottom left
      0.5f,  -0.5f, 0.0f,    1.0f, 0.0f, //bottom right
      0.5f,  0.5f,  0.0f,    1.0f, 1.0f, //top right

      0.5f,  0.5f,  0.0f,    1.0f, 1.0f, //top right
      -0.5f, 0.5f,  0.0f,    0.0f, 1.0f, //top left
      -0.5f, -0.5f, 0.0f,    0.0f, 0.0f, //bottom left
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // texture coordinate atrribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);  // Unbind VAO

    // ------------------- LOAD 1 TEXTURES -------------------
  const char* snowPath = "resources/tex/snowflake.png";
  const char* bgPath = "resources/tex/background.png";
  // variable for textures
  GLuint snowTexture = loadTexture(snowPath);
  // transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  GLuint bgTexture = loadTexture(bgPath);

  // get current time
  float lastTime = glfwGetTime();

  // Game loop
  while (!glfwWindowShouldClose(window)) {
    // get dt of current frame
    float currentTime = glfwGetTime();
    float dt = currentTime - lastTime;
    lastTime = currentTime;

    updateParticles(dt);

    // Check if any events have been activiated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();
    // Render
    // Clear the colorbuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    // Activate shader for bg and snowflake
    bgShader.Use();
    glUniform1i(glGetUniformLocation(bgShader.Program, "bgTexture"), 0);
    glBindVertexArray(bgVAO);
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);
    ourShader.Use();
    GLint texLoc = glGetUniformLocation(ourShader.Program, "uTexture");
    glUniform1i(texLoc, 0);  // tell shader: sampler uses texture unit 0

    // Create transformations
    
    glm::mat4 view(1);
    glm::mat4 projection(1);

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -8.0f));   

    // Note: currently we set the projection matrix each frame, but since the
    // projection matrix rarely changes it's often best practice to set it
    // outside the main loop only once.
    projection = glm::perspective(
        glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

    // Get their uniform locations
    GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
    GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
    GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");

    // set view/projection matrix
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // bind VAO and texture
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, snowTexture);

    // draw all activated particles
    for (int i = 0; i < activeCount; i++)
    {
      Particle& p = particles[i];
      if (!p.active) continue;

      glm::mat4 model(1);
      model = glm::translate(model, p.position);
      model = glm::scale(model, glm::vec3(p.size));
      model = glm::rotate(model, -(GLfloat)glfwGetTime() * glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

      glDrawArrays(GL_TRIANGLES, 0, 6);
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

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
}
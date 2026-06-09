#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Camera.h"

const GLuint WIDTH = 900;
const GLuint HEIGHT = 700;

const GLchar* vertexShaderSource = R"glsl(#version 410 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
out vec4 finalColor;

void main()
{
  gl_Position = projection * view * model * vec4(position, 1.0);
  finalColor = vec4(color, 1.0);
}
)glsl";

const GLchar* fragmentShaderSource = R"glsl(#version 410 core
in vec4 finalColor;
out vec4 color;
uniform float colorBoost;

void main()
{
  vec3 boosted = min(finalColor.rgb * colorBoost, vec3(1.0));
  color = vec4(boosted, 1.0);
}
)glsl";

enum class TransformMode { ROTATE, TRANSLATE, SCALE };
enum class Axis { X, Y, Z };

struct ObjectInstance {
  std::string name;
  GLuint VAO;
  int nVertices;
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  bool valid;
};

std::string resolveObjPath(const std::string& inputPath) {
  std::ifstream direct(inputPath.c_str());
  if (direct.is_open()) {
    return inputPath;
  }

  const std::vector<std::string> candidates = {
      "assets/Modelos3D/" + inputPath,
      "../assets/Modelos3D/" + inputPath,
      "../../assets/Modelos3D/" + inputPath,
      "../../../assets/Modelos3D/" + inputPath,
  };

  for (const auto& candidate : candidates) {
    std::ifstream test(candidate.c_str());
    if (test.is_open()) {
      return candidate;
    }
  }

  return inputPath;
}

GLuint setupShader();
int loadSimpleOBJ(const std::string& filePath, int& nVertices);
void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode);

std::vector<ObjectInstance> gObjects;
int gSelectedIndex = 0;
TransformMode gMode = TransformMode::TRANSLATE;
Axis gAxis = Axis::X;
bool gPerspective = true;

Camera camera(glm::vec3(0.0f, 0.0f, -7.5f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f,
              0.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void printSelectionAndMode() {
  if (gObjects.empty()) {
    return;
  }

  const char* modeName = "TRANSLATE";
  if (gMode == TransformMode::ROTATE) {
    modeName = "ROTATE";
  } else if (gMode == TransformMode::SCALE) {
    modeName = "SCALE";
  }

  const char* axisName = "X";
  if (gAxis == Axis::Y) {
    axisName = "Y";
  } else if (gAxis == Axis::Z) {
    axisName = "Z";
  }

  std::cout << "Selecionado: [" << gSelectedIndex << "] "
            << gObjects[gSelectedIndex].name << " | Modo: " << modeName
            << " | Eixo: " << axisName << std::endl;
}

void applySelectedTransform(GLFWwindow* window, float dt) {
  if (gObjects.empty()) {
    return;
  }

  ObjectInstance& selected = gObjects[gSelectedIndex];
  float direction = 0.0f;
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    direction -= 1.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    direction += 1.0f;
  }

  if (gMode == TransformMode::ROTATE) {
    if (direction != 0.0f) {
      float amount = 90.0f * dt * direction;
      if (gAxis == Axis::X) {
        selected.rotation.x += amount;
      } else if (gAxis == Axis::Y) {
        selected.rotation.y += amount;
      } else {
        selected.rotation.z += amount;
      }
    }
    return;
  }

  if (gMode == TransformMode::SCALE) {
    if (direction != 0.0f) {
      float amount = 1.2f * dt * direction;
      if (gAxis == Axis::X) {
        selected.scale.x = std::max(0.1f, selected.scale.x + amount);
      } else if (gAxis == Axis::Y) {
        selected.scale.y = std::max(0.1f, selected.scale.y + amount);
      } else {
        selected.scale.z = std::max(0.1f, selected.scale.z + amount);
      }
    }
    return;
  }

  if (gMode == TransformMode::TRANSLATE) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      selected.position.x += 1.5f * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      selected.position.x -= 1.5f * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
      selected.position.y += 1.5f * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      selected.position.y -= 1.5f * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
      selected.position.z += 1.5f * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
      selected.position.z -= 1.5f * dt;
    }
  }
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow* window =
      glfwCreateWindow(WIDTH, HEIGHT, "Trabalho Grau A", nullptr, nullptr);
  if (!window) {
    std::cerr << "Erro ao criar janela GLFW" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  GLuint shaderID = setupShader();
  glUseProgram(shaderID);
  glEnable(GL_DEPTH_TEST);

  std::vector<std::pair<std::string, std::string>> toLoad = {
      {"Suzanne", "Suzanne.obj"},
      {"Cube", "Cube.obj"},
      {"SuzanneSubdiv1", "SuzanneSubdiv1.obj"},
  };

  int validObjects = 0;
  for (size_t i = 0; i < toLoad.size(); ++i) {
    ObjectInstance obj;
    obj.name = toLoad[i].first;
    const std::string objPath = resolveObjPath(toLoad[i].second);
    obj.VAO = loadSimpleOBJ(objPath, obj.nVertices);
    obj.valid = (obj.VAO != static_cast<GLuint>(-1));
    obj.position = glm::vec3(2.8f - 2.8f * static_cast<float>(i), 0.0f, 0.0f);
    obj.rotation = glm::vec3(0.0f);
    obj.scale = glm::vec3(1.0f);
    gObjects.push_back(obj);

    if (!obj.valid) {
      std::cerr << "Falha ao carregar objeto: " << toLoad[i].second
                << std::endl;
    } else {
      ++validObjects;
      std::cout << "Objeto carregado: " << obj.name << " (" << objPath << ")"
                << std::endl;
    }
  }

  if (validObjects == 0) {
    std::cerr << "Nenhum objeto valido foi carregado. Encerrando." << std::endl;
    glfwTerminate();
    return -1;
  }

  printSelectionAndMode();

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwPollEvents();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      camera.processKeyboard("FORWARD", deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      camera.processKeyboard("BACKWARD", deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      camera.processKeyboard("LEFT", deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      camera.processKeyboard("RIGHT", deltaTime);
    }

    applySelectedTransform(window, deltaTime);

    glClearColor(0.12f, 0.13f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection;
    if (gPerspective) {
      projection = glm::perspective(
          glm::radians(45.0f),
          static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
    } else {
      projection = glm::ortho(-6.0f, 6.0f, -4.5f, 4.5f, 0.1f, 100.0f);
    }
    glm::mat4 view = camera.getViewMatrix();

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1,
                       GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE,
                       glm::value_ptr(view));

    for (size_t i = 0; i < gObjects.size(); ++i) {
      if (!gObjects[i].valid) {
        continue;
      }

      glm::vec3 renderScale = gObjects[i].scale;
      if (static_cast<int>(i) == gSelectedIndex) {
        renderScale *= 1.2f;
      }

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, gObjects[i].position);
      model = glm::rotate(model, glm::radians(gObjects[i].rotation.x),
                          glm::vec3(1, 0, 0));
      model = glm::rotate(model, glm::radians(gObjects[i].rotation.y),
                          glm::vec3(0, 1, 0));
      model = glm::rotate(model, glm::radians(gObjects[i].rotation.z),
                          glm::vec3(0, 0, 1));
      model = glm::scale(model, renderScale);

      glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE,
                         glm::value_ptr(model));
      glUniform1f(glGetUniformLocation(shaderID, "colorBoost"),
                  static_cast<int>(i) == gSelectedIndex ? 1.9f : 0.7f);

      glBindVertexArray(gObjects[i].VAO);
      glDrawArrays(GL_TRIANGLES, 0, gObjects[i].nVertices);
    }

    glBindVertexArray(0);
    glfwSwapBuffers(window);
  }

  for (const auto& obj : gObjects) {
    if (obj.valid) {
      glDeleteVertexArrays(1, &obj.VAO);
    }
  }

  glfwTerminate();
  return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode) {
  (void)scancode;
  (void)mode;

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  if (action != GLFW_PRESS) {
    return;
  }

  if (key == GLFW_KEY_TAB && !gObjects.empty()) {
    gSelectedIndex = (gSelectedIndex + 1) % static_cast<int>(gObjects.size());
    printSelectionAndMode();
  }

  if (key == GLFW_KEY_R) {
    gMode = TransformMode::ROTATE;
    printSelectionAndMode();
  }

  if (key == GLFW_KEY_T) {
    gMode = TransformMode::TRANSLATE;
    printSelectionAndMode();
  }

  if (key == GLFW_KEY_S) {
    gMode = TransformMode::SCALE;
    printSelectionAndMode();
  }

  if (key == GLFW_KEY_X) {
    gAxis = Axis::X;
    printSelectionAndMode();
  }

  if (key == GLFW_KEY_Y) {
    gAxis = Axis::Y;
    printSelectionAndMode();
  }

  if (key == GLFW_KEY_Z) {
    gAxis = Axis::Z;
    printSelectionAndMode();
  }

  if (key == GLFW_KEY_P) {
    gPerspective = !gPerspective;
  }
}

GLuint setupShader() {
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return shaderProgram;
}

int loadSimpleOBJ(const std::string& filePath, int& nVertices) {
  std::vector<glm::vec3> vertices;
  std::vector<GLfloat> vBuffer;

  std::ifstream input(filePath.c_str());
  if (!input.is_open()) {
    std::cerr << "Erro ao tentar ler o arquivo " << filePath << std::endl;
    return -1;
  }

  std::string line;
  while (std::getline(input, line)) {
    std::istringstream ssline(line);
    std::string word;
    ssline >> word;

    if (word == "v") {
      glm::vec3 vertex;
      ssline >> vertex.x >> vertex.y >> vertex.z;
      vertices.push_back(vertex);
      continue;
    }

    if (word == "f") {
      while (ssline >> word) {
        int vi = 0;
        std::istringstream ss(word);
        std::string index;
        if (std::getline(ss, index, '/')) {
          vi = !index.empty() ? std::stoi(index) - 1 : 0;
        }

        if (vi < 0 || vi >= static_cast<int>(vertices.size())) {
          continue;
        }

        vBuffer.push_back(vertices[vi].x);
        vBuffer.push_back(vertices[vi].y);
        vBuffer.push_back(vertices[vi].z);

        vBuffer.push_back((rand() % 256) / 255.0f);
        vBuffer.push_back((rand() % 256) / 255.0f);
        vBuffer.push_back((rand() % 256) / 255.0f);
      }
    }
  }

  input.close();

  if (vBuffer.empty()) {
    std::cerr << "OBJ sem vertices processaveis: " << filePath << std::endl;
    return -1;
  }

  GLuint VBO = 0;
  GLuint VAO = 0;

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat),
               vBuffer.data(), GL_STATIC_DRAW);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                        (GLvoid*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                        (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  nVertices = static_cast<int>(vBuffer.size() / 6);
  return static_cast<int>(VAO);
}

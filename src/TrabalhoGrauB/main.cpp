#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fileReader.hpp>
#include <cmath>
#include <shader.hpp>
#include <camera.hpp>
#include <model.hpp>

#include <material.hpp>
#include <light.hpp>
#include <skybox.hpp>
#include <curve.hpp>
#include <scene.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double, double);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint loadSkybox(std::vector<std::string> faces);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0, 0.0, 3.0));

bool orthographic = false;
glm::mat4 ortho = glm::ortho(
		-4.0f, 4.f, -3.f, 3.0f, 0.1f, 100.f);
glm::mat4 perspective =
			glm::perspective(
					glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);

float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime=0.0, lastFrame = 0.0;

Shader* shaderGlobal;
int currentModel = 0;
int currentLightDist = 0;
std::vector<Model> modelList; 

PointLight light = {
	.position=glm::vec3(1.2, 1.0, 2.0),
	.ambient=glm::vec3(1.0f),
	.diffuse=glm::vec3(1.0f),
	.specular=glm::vec3(1.0f),
	.distance = LIGHT_DISTANCES[currentLightDist]
};

int main() {
	

	if (!glfwInit()) return -1;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "learn opengl", NULL, NULL);
	
	if (window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glEnable(GL_DEPTH_TEST);
	
	Scene scene = Scene("./scene.json");

	modelList = scene.models;
	camera = scene.camera;
	perspective = scene.projection;

	std::vector<std::string> faces
	{
			"./assets/skybox/right.jpg",
			"./assets/skybox/left.jpg",
			"./assets/skybox/top.jpg",
			"./assets/skybox/bottom.jpg",
			"./assets/skybox/front.jpg",
			"./assets/skybox/back.jpg"
	};
	GLuint skyboxTex = loadSkybox(faces);

	Shader shader("./shaders/vertex.glsl", "./shaders/fragment.glsl", "./shaders/geometry.glsl");
	Shader lightShader("./shaders/lightVertex.glsl", "./shaders/lightFragment.glsl");
	Shader skyboxShader("./shaders/skybox.vert", "./shaders/skybox.frag");
	shaderGlobal = &shader;
	
	skyboxShader.use();
	skyboxShader.setInt("skybox", 1);
	
	Skybox skybox;

	shader.use();
	shader.setInt("skybox", 1);
	shader.setInt("pointLightSize", scene.pointLights.size());
	shader.setInt("spotLightSize", scene.spotLights.size());

	for (int i = 0; i < scene.pointLights.size(); i++) {
		shader.setPointLight(scene.pointLights[i], i);
	}
	for (int i = 0; i < scene.spotLights.size(); i++) {
		shader.setSpotLight(scene.spotLights[i], i);
	}

	shader.setDirLight(scene.dirLight);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);


	shader.setVec3f("wireframeColor", glm::vec3(0.0f, 1.0f, 0.0f));
	shader.setFloat("wireframeWidth", 0.005f);
	shader.setBool("wireframe", false);
	
	Model lightCube = Model("./assets/Modelos3D/Cube.obj", light.position);

	while(!glfwWindowShouldClose(window)){
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		shader.use();
		
		glm::mat4 projection = (orthographic) ? ortho : perspective;

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
		shader.setMatrix4f("projection", projection);
		shader.setMatrix4f("view", view);
		shader.setVec3f("viewPos", camera.Position);

		for(int i = 0; i < modelList.size(); i++) {
			modelList[i].Draw(shader, deltaTime);
		}
		
		lightShader.use();
		lightShader.setMatrix4f("projection", projection);
		lightShader.setMatrix4f("view", view);
		for (int i = 0; i < scene.pointLights.size(); i++) {
			lightCube.DrawLight(lightShader, scene.pointLights[i].position);
		}
		for (int i = 0; i < scene.spotLights.size(); i++) {
			lightCube.DrawLight(lightShader, scene.spotLights[i].position);
		}
		
		skyboxShader.use();
		skyboxShader.setMatrix4f("projection", perspective);
		skyboxShader.setMatrix4f("view", skyboxView);
		skybox.draw();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow *window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}

	
	/*
		NumPad Controls
		- (select last Model from modelList) + (select next Model from modelList)
		7 (set Operation TRANSLATE to current Model) 8 9 (Sub X Add X) 
		4 (set Operation ROTATE to current Model)    5 6 (Sub Y Add Y)
		1 (set Operation SCALE to current Model)     2 3 (Sub Z Add Z)
		0 (set Operation SCALE_SIMETRICALLY to current Model)
		. (reset Scaling and Rotation of current Model)
		/ (set Wireframe ON and OFF)
		NM Cycle through materials on the currentModel
		KL Increase Decrease Light Distance
	*/
	
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
		modelList[currentModel].moveModel(X_AXIS, -deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
		modelList[currentModel].moveModel(X_AXIS, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
		modelList[currentModel].moveModel(Y_AXIS, -deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		modelList[currentModel].moveModel(Y_AXIS, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		modelList[currentModel].moveModel(Z_AXIS, -deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		modelList[currentModel].moveModel(Z_AXIS, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		modelList[currentModel].setOperation(TRANSLATE);
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		modelList[currentModel].setOperation(ROTATE);
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		modelList[currentModel].setOperation(SCALE);
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		modelList[currentModel].setOperation(SCALE_SIMETRICAL);
	}

	if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
		modelList[currentModel].resetModelScaleAndRotation();
	}

}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	static bool wireframe = false;
	if (key ==  GLFW_KEY_EQUAL && action == GLFW_PRESS){
		currentModel = std::min(++currentModel, static_cast<int>(modelList.size() - 1));
		modelList[currentModel].setOperation(TRANSLATE);
		std::cout << currentModel << std::endl;
	}
	if (key == GLFW_KEY_MINUS && action == GLFW_PRESS){
		currentModel = std::max(0, --currentModel);
		modelList[currentModel].setOperation(TRANSLATE);
		std::cout << currentModel << std::endl;
	}
	if (key == GLFW_KEY_SLASH && action == GLFW_PRESS){
		wireframe ^= 1;
		shaderGlobal->use();
		shaderGlobal->setBool("wireframe", wireframe);
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS){
		orthographic ^= 1;
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS){
		modelList[currentModel].cycleMaterial(true);
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS){
		modelList[currentModel].cycleMaterial(false);
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS){
		currentLightDist = std::min(++currentLightDist, static_cast<int>(LIGHT_DISTANCES.size() - 1));
		light.distance = LIGHT_DISTANCES[currentLightDist];
	}
	if (key == GLFW_KEY_K && action == GLFW_PRESS){
		currentLightDist = std::max(0, --currentLightDist);
		light.distance = LIGHT_DISTANCES[currentLightDist];
	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
	perspective =
			glm::perspective(
					glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

unsigned int loadSkybox(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
					);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

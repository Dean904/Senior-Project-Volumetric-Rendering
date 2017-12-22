// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW\glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// ETC
#include <iostream>
#include <Shader.cpp>
#include <Camera.cpp>
#include <Model.cpp>
#include <LightModel.cpp>
#include <ChunkManager.cpp>
#include <Chunk.cpp>
#include <SquareModelStd.cpp>
#include <Animator.cpp>

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void Do_Movement();
void miscKeybinds();

// Window dimensions
const GLuint screenWidth = 800, screenHeight = 600;

// Input & Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
bool leftMouseDown = false;
bool rightMouseDown = false;
GLfloat lastMouseX = 400, lastMouseY = 300;
bool firstMouseMovement = true;

glm::mat4 view;
glm::mat4 projection;

glm::vec3 rayStart;
glm::vec3 rayEnd;

// Global Variables
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lastPos; // last camera position (debug var)
GLfloat timeOfLastQuery = 0;

bool polygonModeEnabled = false;
int renderingMode = 0;
bool currentlyCastingRays = false;

Animator animator;
ChunkManager chunkManager;
std::vector<glm::vec3> raypoints;


// define static member
SquareModelStd Chunk::square;

int main()
{
	// Window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Ascension", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create a window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW." << std::endl;
		return -1;
	}

	// Viewport
	glViewport(0, 0, screenWidth, screenHeight);

	// Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// Locks cursor to the screen
	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LESS);			// options necessary for stencil / outline 
//	glEnable(GL_STENCIL_TEST);
//	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
//	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//__OpenGL_____________
	// Build and compile our shaders
	Shader ourShader("basicVertex.vert", "basicFragment.frag");
	Shader lampShader("basicVertex.vert", "lampFrag.txt");
	//Shader stencilShader("../Samsara Alpha/basicVertex.vert", "../Samsara Alpha/basicFragment.frag");
						//("../Ascension/stencilVertex.txt", "../Ascension/stencilFrag.txt")

	// Misc Code
	GLint nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum # of vertex attributes supported: " << nrAttributes << std::endl;

	GLfloat currentFrame = glfwGetTime();
	//Model ourModel = Model("../Samsara Alpha/rsc/~animals/Wolf/Wolf.obj");
	Model ourModel1 = Model("../Samsara Alpha/rsc/~animals/Bison/Bison.obj");
	//Model ourModel2 = Model("../Ascension/rsc/~animals/Deer/Deer.obj");
	//Model ourModel3 = Model("../Ascension/rsc/~animals/lowpolycat/cat.obj");
	//Model ourModel5 = Model("../Ascension/rsc/~animals/lowpolycow/cow.obj");
	//Model ourModel6 = Model("../Ascension/rsc/~animals/lowpolydeer/deer.obj");
	//Model ourModel7 = Model("../Ascension/rsc/~animals/lowpolywolf/wolf.obj");

	//Model weapon0 = Model("C:/Users/Dean/Documents/Visual Studio 2017/Projects/Ascension/Ascension/rsc/~weapons/Dagger4/dagger4_obj.obj");
	//Model weapon1 = Model("../Ascension/rsc/~weapons/FantasySword/Magic_Sword.obj");
	//Model weapon2 = Model("../Ascension/rsc/~weapons/Giantsword/GiantSword.blend");
	//Model weapon3 = Model("../Ascension/rsc/~weapons/SandrasSword/SandrasSword.obj");
	//Model weapon4 = Model("../Ascension/rsc/~weapons/Sword/SwordGame.obj");

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-12.0f,  -5.0f, -10.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	LightModel lightModel0 = LightModel(pointLightPositions[0]);	// Implement Cloning!!!
	LightModel lightModel1 = LightModel(pointLightPositions[1]);
	LightModel lightModel2 = LightModel(pointLightPositions[2]);
	LightModel lightModel3 = LightModel(pointLightPositions[3]);

	std::cout << "Meshes loaded in " << currentFrame - lastFrame << " seconds" << std::endl;

	chunkManager.initialize();
	animator = Animator(&camera, glm::vec3((chunkManager.CHUNK_NUM * Chunk::CHUNK_SIZE) / 2));

	//__Game Loop__________
	while (!glfwWindowShouldClose(window))
	{
		// Time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and Call events
		glfwPollEvents();
		Do_Movement();
		animator.tick(deltaTime, glm::vec3(camera.Position));

		// Rendering
		// Clear the color buffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// GL_STENCIL_BUFFER_BIT

		// Configure Shader
		ourShader.Use();
		//ourShader.setVec3("lightColor", 1.0f, 0.5f, 1.0f);	// do these calls have to be done in while loop?

		// Set shader lighting info
		ourShader.setVec3("viewPos", camera.Position);

		ourShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
		ourShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
		ourShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		ourShader.setFloat("material.shininess", 2.0f);

		glm::vec3 lightColor;
		lightColor.x = 255.0f;
		lightColor.y = 255.0f;
		lightColor.z = 255.0f;

		glm::vec3 diffuseColor = lightColor * glm::vec3(0.0005f); // decrease the influence
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence

		ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		ourShader.setVec3("dirLight.ambient", ambientColor);
		ourShader.setVec3("dirLight.diffuse", diffuseColor);
		ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

		ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		ourShader.setFloat("pointLights[0].constant", 1.0f);	// reduce calls by setting default values?
		ourShader.setFloat("pointLights[0].linear", 0.09f);
		ourShader.setFloat("pointLights[0].quadratic", 0.032f);
		ourShader.setVec3("pointLights[0].ambient", ambientColor);
		ourShader.setVec3("pointLights[0].diffuse", diffuseColor);
		ourShader.setVec3("pointLights[0].specular", 0.5f, 0.5f, 0.5f);

		ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		ourShader.setFloat("pointLights[1].constant", 1.0f);	// reduce calls by setting default values?
		ourShader.setFloat("pointLights[1].linear", 0.09f);
		ourShader.setFloat("pointLights[1].quadratic", 0.032f);
		ourShader.setVec3("pointLights[1].ambient", ambientColor);
		ourShader.setVec3("pointLights[1].diffuse", diffuseColor);
		ourShader.setVec3("pointLights[1].specular", 0.5f, 0.5f, 0.5f);

		ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
		ourShader.setFloat("pointLights[2].constant", 1.0f);	// reduce calls by setting default values?
		ourShader.setFloat("pointLights[2].linear", 0.09f);
		ourShader.setFloat("pointLights[2].quadratic", 0.032f);
		ourShader.setVec3("pointLights[2].ambient", ambientColor);
		ourShader.setVec3("pointLights[2].diffuse", diffuseColor);
		ourShader.setVec3("pointLights[2].specular", 0.5f, 0.5f, 0.5f);

		ourShader.setVec3("pointLights[3].position", pointLightPositions[3]);
		ourShader.setFloat("pointLights[3].constant", 1.0f);	// reduce calls by setting default values?
		ourShader.setFloat("pointLights[3].linear", 0.09f);
		ourShader.setFloat("pointLights[3].quadratic", 0.032f);
		ourShader.setVec3("pointLights[3].ambient", ambientColor);
		ourShader.setVec3("pointLights[3].diffuse", diffuseColor);
		ourShader.setVec3("pointLights[3].specular", 0.5f, 0.5f, 0.5f);

		view = camera.GetViewMatrix();
		ourShader.setMat4("view", view);
		projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 500.0f);;
		ourShader.setMat4("projection", projection);

		glm::mat4 model;
		//ourShader.setMat4("model", model);


		// set stencil mask to 0x00 to not write to the stencil buffer.
		//glStencilMask(0x00);

		//glEnable(GL_CULL_FACE);	// requires normal vectors
		//glCullFace(GL_BACK);

		//glm::mat4 model;
		model = glm::mat4();
		//model = glm::translate(model, glm::vec3(x, y, z));
		model = glm::translate(model, glm::vec3(-10, -10, -10));
		model = glm::scale(model, glm::vec3(0.4f)); // Make it smaller 
		ourShader.setMat4("model", model);
		//weapon0.Draw();
		//weapon0.printVertices();

		// bison
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-10, 5, -10));
		model = glm::scale(model, glm::vec3(5.0f));  
		ourShader.setMat4("model", model);
		ourModel1.Draw();

		//ourModel1.voxelize();
		//ourModel1.setPosition(glm::vec3(-40, 5, -7));
		//ourModel1.Draw(ourShader);

		// wolf
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-15, 5, -15));
		model = glm::scale(model, glm::vec3(0.05f)); // Make it smaller 
		ourShader.setMat4("model", model);
		//ourModel.Draw();

		//ourModel.voxelize();
		//ourModel.setPosition(glm::vec3(-60, 5, -30));
		//ourModel.Draw(ourShader);

		//ourModel7.printVertices();
		
		// Raycasting code
		int mouseY = screenHeight - lastMouseY;
		rayStart = glm::unProject(glm::vec3(lastMouseX, mouseY, 0.0f), camera.GetViewMatrix(), projection, glm::vec4(0, 0, screenWidth, screenHeight));
		rayEnd = glm::unProject(glm::vec3(lastMouseX, mouseY, 1.0f), camera.GetViewMatrix(), projection, glm::vec4(0, 0, screenWidth, screenHeight));
		//glm::vec3 rayDir = glm::normalize(rayEnd - rayStart);
		if (renderingMode == 0) chunkManager.MarchingCubesDraw(ourShader);
		else if (renderingMode == 1) chunkManager.Draw(ourShader);
		else if (renderingMode == 2) 
		{
			chunkManager.drawOctree(ourShader);
		}
		//else if (renderingMode == 3) chunkManager.octree_raycast(rayStart, rayDir, ourShader);
		else if (renderingMode == 3)
		{
			//chunkManager.MarchingCubesDraw(ourShader);
			chunkManager.drawFullRaycast(ourShader);		
			chunkManager.renderRaycast(ourShader);
		}



		//Chunk::square.Draw(ourShader, -5, -3, -4);
		//Chunk::square.Draw(ourShader, -5, 3, 4);



		//std::cout << "Raystart [" << rayStart.x << "," << rayStart.y << "," << rayStart.z << "] \n";
		//std::cout << "Pos [" << camera.Position.x << "," << camera.Position.y << "," << camera.Position.z << "] \n";

		//rayDir = glm::normalize(rayEnd - rayStart);

		

		//std::cout << "RayStart Vector: (" << rayStart.x << ", " << rayStart.y << ", " << rayStart.z << ")" << std::endl;
		//std::cout << "RayEnd Vector: (" << rayEnd.x << ", " << rayEnd.y << ", " << rayEnd.z << ")" << std::endl;
		//std::cout << "RayDir Vector: (" << rayDir.x << ", " << rayDir.y << ", " << rayDir.z << ")" << std::endl << std::endl;

		model = glm::mat4();
		model = glm::scale(model, glm::vec3(0.4f));

		//for(glm::vec3 v : raypoints) {
		//	Chunk::square.Draw(ourShader, v.x, v.y, v.z);
		//}

		//SquareModelStd stdSquare = SquareModelStd(glm::vec3(-3, -3, -3));
		//stdSquare.Draw(ourShader);

		

		lampShader.Use();
		lampShader.setMat4("view", view);
		lampShader.setMat4("projection", projection);

		lightModel0.Draw(lampShader);
		lightModel1.Draw(lampShader);
		lightModel2.Draw(lampShader);
		lightModel3.Draw(lampShader);


		glfwSwapBuffers(window);

	}

	// Properly de-allocate all resources once they've outlived their purpose
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;
}


// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (leftMouseDown && rightMouseDown)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_SPACE])
		camera.ProcessKeyboard(UP, deltaTime);
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);

}

void miscKeybinds(int key, int action)
{
	if (key == GLFW_KEY_F)
		std::cout << "FPS: " << (1 / deltaTime) << std::endl;

	if (key == GLFW_KEY_R)
		chunkManager.manipulateVoxelWithRay(rayStart, glm::normalize(rayEnd - rayStart), false);

	if (key == GLFW_KEY_T)
		chunkManager.manipulateVoxelWithRay(rayStart, glm::normalize(rayEnd - rayStart), true);

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		chunkManager.computerFullRaycast(camera.GetViewMatrix(), projection, screenWidth, screenHeight);
	}

	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		animator.startCirculationAnimation();
		camera.move(glm::vec3(0));
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		chunkManager.octree_raycast(rayStart, glm::normalize(rayEnd - rayStart), true);
	}

	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		// Output positions
		//if (lastPos.x != (int)camera.Position.x || lastPos.y != (int)camera.Position.y || lastPos.z != (int)camera.Position.z)
			std::cout << (int)camera.Position.x << ", " << (int)camera.Position.y << ", " << (int)camera.Position.z << std::endl;
		//lastPos.x = (int)camera.Position.x;
		//lastPos.y = (int)camera.Position.y;
		//lastPos.z = (int)camera.Position.z;
	}
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		if (polygonModeEnabled)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			polygonModeEnabled = false;
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			polygonModeEnabled = true;
		}
	}

	if (key == GLFW_KEY_B && action == GLFW_PRESS)
	{
		if (renderingMode < 3) {
			renderingMode++;
		}
		else {
			renderingMode = 0;
		}
	}

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Is called whenever a key is pressed/released via GLFW
	//std::cout << key << std::endl;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	miscKeybinds(key, action);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)

{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
			leftMouseDown = true;
		else if (action == GLFW_RELEASE)
			leftMouseDown = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
			rightMouseDown = true;
		else if (action == GLFW_RELEASE)
			rightMouseDown = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouseMovement)
	{
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouseMovement = false;
	}

	GLfloat xoffset = xpos - lastMouseX;
	GLfloat yoffset = lastMouseY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastMouseX = xpos;
	lastMouseY = ypos;

	if (leftMouseDown) {
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

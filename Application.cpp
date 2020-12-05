#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"uniform mat4 transform;\n"
"out vec4 vertexColor;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"   gl_Position = transform * vec4(aPos, 1.0);\n"
"	vertexColor = vec4(aColor, 1.0);\n"
"	TexCoord = aTexCoord;\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec4 vertexColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D texture1;\n"
"uniform sampler2D texture2;\n"
"uniform float mixValue;\n"
"uniform float colorValue;\n"
"void main() {\n"
"	FragColor = mix(mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue), vertexColor, colorValue/2);\n"
"}\0";

const float MIN_X_OFFSET = -0.4f;
const float MIN_Y_OFFSET = -0.4f;
const float MAX_X_OFFSET = 0.4f;
const float MAX_Y_OFFSET = 0.4f;
const float MIN_SCALE = 0.1f;
const float MAX_SCALE = 1.0f;
const float PI = 3.1415926535897932384f;

float mixValue;
float colorValue;
float xOffset;
float yOffset;
float xRotate;
float yRotate;
float zRotate;
float currentScale;


int main(void) {
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(2400, 1800, "ControllablePlane", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}


	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glewInit();

	//Setup callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Setup triangle
	float vertices[] = {
		 // positions         // colors            // texture coords
		 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  //top right
		 0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,  //bottom right
		-0.5f, -0.5f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  //bottom left
		-0.5f,  0.5f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f   //top left
	};

	unsigned int indices[] = {
		0, 1, 3, //First triangle
		1, 2, 3 //Second triangle
	};

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//Finally, the vertex attrib code!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3*sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	//Shader setup
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//Shader error checking
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//Shader program creation
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//Shader program error checking
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}
	
	glUseProgram(shaderProgram);

	//Shader objects are unnecessary one program is successfully linked so we clean them up here
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Texturing
	float texCoords[] = {
		0.0f, 0.0f,  //lower-left corner
		1.0f, 0.0f,  //lower-right corner
		0.5f, 1.0f   //top-center corner
	};
	
	//STBI config to make images not upside down
	stbi_set_flip_vertically_on_load(true);

	//Setup texture 1
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);

	//Set texture wrapping/filtering options on the currently bound texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//Setup texture 2
	unsigned int texture2;
	glGenTextures(1, &texture2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);

	//Set texture wrapping/filtering options on the currently bound texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//Transformations!
	glm::vec4 vec(1.0, 0.0, 0.0, 1.0);
	glm::mat4 trans(1.0f);

	vec = trans * vec;

	//Finally, set up uniforms
	mixValue = 0.2f;
	colorValue = 0.5f;
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);
	glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);
	glUniform1f(glGetUniformLocation(shaderProgram, "colorValue"), colorValue);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(trans));

	//Initialize necessary values
	xOffset = 0;
	yOffset = 0;
	currentScale = 1.0;
	xRotate = 0;
	yRotate = 0;
	zRotate = 0;

	//Render Loop
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		processInput(window);
		glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);
		glUniform1f(glGetUniformLocation(shaderProgram, "colorValue"), colorValue);

		//Rotate the square
		trans = glm::mat4(1.0f);
		trans = glm::translate(trans, glm::vec3(xOffset, yOffset, 0.0));
		//Three rotations; one for each axis, separated because a zero axis breaks the graphics.
		trans = glm::rotate(trans, xRotate, glm::vec3(1, 0, 0));
		trans = glm::rotate(trans, yRotate, glm::vec3(0, 1, 0));
		trans = glm::rotate(trans, zRotate, glm::vec3(0, 0, 1));

		trans = glm::scale(trans, glm::vec3(currentScale, currentScale, currentScale));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(trans));

		//Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, 3);

		//Check/Call events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//GL Cleanup
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	//GLFW Cleanup
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		mixValue += 0.01f;
		if (mixValue >= 1.0f) {
			mixValue = 1.0f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		mixValue -= 0.01f;
		if (mixValue <= 0.00f) {
			mixValue = 0.0f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		colorValue += 0.01f;
		if (colorValue >= 1.0f) {
			colorValue = 1.0f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		colorValue -= 0.01f;
		if (colorValue <= 0.00f) {
			colorValue = 0.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		yOffset += 0.01;
		if (yOffset >= MAX_Y_OFFSET) {
			yOffset = MAX_Y_OFFSET;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		xOffset -= 0.01;
		if (xOffset <= MIN_X_OFFSET) {
			xOffset = MIN_X_OFFSET;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		yOffset -= 0.01;
		if (yOffset <= MIN_Y_OFFSET) {
			yOffset = MIN_Y_OFFSET;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		xOffset += 0.01;
		if (xOffset >= MAX_X_OFFSET) {
			xOffset = MAX_X_OFFSET;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		currentScale -= 0.01;
		if (currentScale <= MIN_SCALE) {
			currentScale = MIN_SCALE;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		currentScale += 0.01;
		if (currentScale >= MAX_SCALE) {
			currentScale = MAX_SCALE;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		xRotate += 0.02;
		if (xRotate >= 2 * PI) {
			xRotate = xRotate - 2 * PI;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		xRotate -= 0.02;
		if (xRotate <= -2 * 3.14159) {
			xRotate = xRotate + 2 * PI;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		yRotate += 0.02;
		if (yRotate >= 2 * 3.14159) {
			yRotate = yRotate - 2 * PI;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		yRotate -= 0.02;
		if (yRotate <= -2 * 3.14159) {
			yRotate = yRotate + 2 * PI;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		zRotate += 0.02;
		if (zRotate >= 2 * 3.14159) {
			zRotate = zRotate - 2 * PI;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		zRotate -= 0.02;
		if (zRotate <= -2 * 3.14159) {
			zRotate = zRotate + 2 * PI;
		}
	}
}
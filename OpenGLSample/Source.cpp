#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

//Source code for shaders
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);\n"
"}\n\0";




int main() {
	glfwInit();

	//Give a "hint" of what OpenGL configuration to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//We are using the "core" profile, which means only modern OpenGL functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Specify the points of the shapes
	GLfloat vertices[] = {
		0.0f, -0.5f, 0.0f,     //Bottom Middle
		0.7f, -0.5f, 0.0f,       //Bottom Right
		0.0f, 0.7f, 0.0f,        //Left's Top
		0.7f, 0.7f, 0.0f,        //Right's Top
		-0.7f, -0.5f, 0.0f      //Bottom Left
	};

	//In what order do we visit vertices?
	GLuint indices[] = {
		0, 1, 3,  //Right Triangle
		0, 4, 2   //Left Triangle
	};

	GLFWwindow* window = glfwCreateWindow(800, 800, "Window of Justice", NULL, NULL);
	if (window == NULL) {
		std::cout << "Your implementation of GLFW can't even create a window."
			"What are you doing with your life?" << std::endl;
		//Don't just let things keep running when you're done with them. It could make problems 
		glfwTerminate();
		return -1;
	}
	// The context is like what is happening now. This makes the window happen now
	glfwMakeContextCurrent(window);
	
	//glad helps OpenGL get configured
	gladLoadGL();

	// The viewport is the "interior" of the window. It's where the action happens. Here, we size it
	glViewport(0, 0, 800, 800);

	//Create shader objects and references
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//Attach shader source to object
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, & fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);


	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Create containers for array and buffer objects. Buffer is the chunk of vertices you send to GPU
	GLuint VAO, VBO, EBO;

	// Generate VAO, VBO, and EBO with 1 object each
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	//Make the new VAO the current context
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Place verices in the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	// Configure so that OpenGL can read the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	// These 2 are common functions. It clears the background color and replaces it with this color
	glClearColor(0.89f, 0.81f, 0.89f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	// There are 2 buffers, which are like windows. One is being displayed while the other is
	// being overwritten. They must be swapped when the overwrite is finished. This is the swap.
	glfwSwapBuffers(window);



	// render loop
	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.89f, 0.81f, 0.89f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}
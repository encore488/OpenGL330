#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

int main() {
	glfwInit();

	//Give a "hint" of what OpenGL configuration to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//We are using the "core" profile, which means only modern OpenGL functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	// These 2 are common functions. It clears the background color and replaces it with this color
	glClearColor(0.89f, 0.81f, 0.89f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	// There are 2 buffers, which are like windows. One is being displayed while the other is
	// being overwritten. They must be swapped when the overwrite is finished. This is the swap.
	glfwSwapBuffers(window);

	// render loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}
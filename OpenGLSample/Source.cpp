#include <iostream>  // g++ main.cpp -lGL -lGLU -lglut -lGLEW -lglfw
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
//GLM Math Headers
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Define global variables for each shapes VAO and index count
GLuint pyramidVAO;
GLuint pyramidVBO;
GLuint pyramidEBO;
GLuint pyramidIndicesCount;

GLuint cubeVAO;
GLuint cubeIndicesCount;


namespace
{
    const char* const WINDOW_TITLE = "Window of Justice";
    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 850;

    //A public class that stores the data for a mesh
    //A mesh is a collection of vertices, faces, and edges to define the shape of objects in 3d space
    struct GLMesh
    {
        GLuint vao;         //Vertex Array Object
        GLuint vbos[2];     //Vertex Buffer Objects
        GLuint nIndices;
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Shader program
    GLuint gProgramId;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


//Vertex Shader Source Code
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
out vec4 vertexColor;

//Globals for transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vertexColor = color;
});


//Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
    //Hold incoming color data from vertex shader
    in vec4 vertexColor;
out vec4 fragmentColor;
void main()
{
    fragmentColor = vec4(vertexColor);
});



int main(int argc, char* argv[])
{
    //Initialize program
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //Create empty GLmesh, with empty vao, 2 vbos and nIndices
    UCreateMesh(gMesh);

    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    //Clear color and set a new one
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop, aka game loop
    while (!glfwWindowShouldClose(gWindow))
    {

        UProcessInput(gWindow);

        URender();

        glfwPollEvents();
    }

    //Destroy stuff when you're done. Who knows what would happen if you just let it keep going?
    UDestroyMesh(gMesh);
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS);
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "You can't even create a GLFW window? Wow." << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


//query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Pyramid transformations
    glm::mat4 pyramidModel = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)); // Translate the pyramid to the left
    glm::mat4 pyramidView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));  // Your view matrix for the pyramid
    glm::mat4 pyramidProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);  // Your projection matrix for the pyramid

    //Retreive and pass transformation matrices to the shader
    GLint pyrmodelLoc = glGetUniformLocation(gProgramId, "model");
    GLint pyrviewLoc = glGetUniformLocation(gProgramId, "view");
    GLint pyrprojLoc = glGetUniformLocation(gProgramId, "projection");

    // Render the Pyramid
    glBindVertexArray(pyramidVAO);
    glUniformMatrix4fv(pyrmodelLoc, 1, GL_FALSE, glm::value_ptr(pyramidModel));
    glUniformMatrix4fv(pyrviewLoc, 1, GL_FALSE, glm::value_ptr(pyramidView));
    glUniformMatrix4fv(pyrprojLoc, 1, GL_FALSE, glm::value_ptr(pyramidProjection));
    glDrawElements(GL_TRIANGLES, pyramidIndicesCount, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    // Cube transformations
    glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));  // Translate the cube to the right
    glm::mat4 cubeView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));  // Your view matrix for the cube
    glm::mat4 cubeProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);  // Your projection matrix for the cube

    //Retreive and pass transformation matrices to the shader
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    // Render the Cube
    glBindVertexArray(cubeVAO);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cubeView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cubeProjection));
    glDrawElements(GL_TRIANGLES, cubeIndicesCount, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    // Flips the back buffer with the front buffer every frame
    glfwSwapBuffers(gWindow);
}



void UCreateMesh(GLMesh& mesh)
{

    // // // //     Create Cube     // // // //
    // Define vertices for the cube
    GLfloat vertsA[] = {
        // Vertex Positions    // Colors (r,g,b,a)
        0.5f,  1.5f, -0.5f,   0.7f, 0.2f, 0.2f, 1.0f, // Back Top Right Vertex 0
         0.5f, -0.5f, 0.0f,   0.2f, .7f, 0.2f, 1.0f, // Front Bottom Right 1
        -0.5f, -0.5f, 0.0f,   0.2f, 0.2f, 0.7f, 1.0f, // Front Bottom Left 2
         0.5f, -0.5f, -1.0f,  0.5f, 0.5f, 1.0f, 1.0f, // Back Bottom Right 3
        -0.5f, -0.5f, -1.0f,  0.7f, 0.2f, 0.7f, 1.0f,  // Back Bottom Left 4
        0.5f,  1.5f, 0.5f,   0.7f, 0.2f, 0.2f, 1.0f, // Front Top Right Vertex 5
        -0.5f,  1.5f, -0.5f,   0.7f, 0.2f, 0.2f, 1.0f, // Back Top Left Vertex 6
        -0.5f,  1.5f, 0.5f,   0.7f, 0.2f, 0.2f, 1.0f, // Front Top Left Vertex 7
    };

    // Define indices for the cube
    GLushort indicesA[] = {
        0, 1, 3,  // Triangle 1 (right side)
        0, 5, 1,   // Triangle 2 (right side)
        2, 4, 6,  // Triangle 3 (left side)
        2, 7, 6,  // Triangle 4 (left side)
        1, 2, 3, // Triangle 5 (bottom)
        3, 4, 2,  // Triangle 6 (bottom)
        5, 6, 7,  // Triangle 7 (top)
        5, 0, 6,  // Triangle 8 (top)
        0, 3, 4,  // Triangle 9 (back)
        0, 6, 3,  // Triangle 10 (back)
        1, 2, 5,  // Triangle 11 (front)
        2, 7, 5,  // Triangle 12 (front)
    };

    // Create VAO and buffers for the cube
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    glGenBuffers(1, &pyramidEBO); // Create 2 buffers for the cube
    glBindBuffer(GL_ARRAY_BUFFER, pyramidEBO); // Activate the buffer for vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsA), vertsA, GL_STATIC_DRAW); // Send vertex data to the GPU

    cubeIndicesCount = sizeof(indicesA) / sizeof(indicesA[0]);


    // // // //     Create Pyramid     // // // //
    // Specifies Normalized Device Coordinates (x,y,z) and color (r,g,b,a) for vertices
    GLfloat vertsB[] =
    {
        // Vertex Positions    // Colors (r,g,b,a)
         0.0f,  1.4f, -0.5f,   0.7f, 0.2f, 0.2f, 1.0f, // Top Vertex 0
         0.5f, -0.5f, 0.0f,   0.2f, .7f, 0.2f, 1.0f, // Bottom Right 1
        -0.5f, -0.5f, 0.0f,   0.2f, 0.2f, 0.7f, 1.0f, // Bottom Left 2
         0.5f, -0.5f, -1.0f,  0.5f, 0.5f, 1.0f, 1.0f, // Back Bottom Right 3
        -0.5f, -0.5f, -1.0f,  0.7f, 0.2f, 0.7f, 1.0f  // Back Bottom Left 4
    };

    // Index data to share position data
    GLushort indicesB[] = {
        0, 1, 3,  // Triangle 1 (left side)
        0, 2, 4,   // Triangle 2 (right side)
        0, 1, 2,  // Triangle 3 (front side)
        0, 3, 4,  // Triangle 4 (back side)
        3, 4, 2, // Triangle 5 (bottom)
        1, 2, 3,  // Triangle 6 (bottom)
    };
    // Create separate VBO and EBO for the pyramid
    glGenBuffers(2, &pyramidVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsB), vertsB, GL_STATIC_DRAW);

    glGenVertexArrays(1, &pyramidVAO);
    glBindVertexArray(pyramidVAO);

    glGenBuffers(1, &pyramidEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesB), indicesB, GL_STATIC_DRAW);

    pyramidIndicesCount = sizeof(indicesB) / sizeof(indicesB[0]);



    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;


    //Strides between vertex coordinates: (x, y, z, r, g, b, a). Tightly packed strides are 0
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);


    // Creates the Vertex Attribute Pointer
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbos[0]); // Update this line to delete the correct buffer
    glDeleteBuffers(1, &mesh.vbos[1]); // Update this line to delete the correct buffer

}


bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrieve the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile vertex shader and print errors
    glCompileShader(vertexShaderId);
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId);
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attach compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   //Link the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
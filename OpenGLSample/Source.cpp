#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <GL/GLU.h>
#include "camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>




using namespace std;

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

#define SCREEN_WIDTH 1100
#define SCREEN_HEIGHT 800

namespace
{
    const char* const WINDOW_TITLE = "Window of Justice";
    const int WINDOW_WIDTH = SCREEN_WIDTH;
    const int WINDOW_HEIGHT = SCREEN_HEIGHT;

    // 4*4 matrices for transforming shapes' vertices into clip coordinates
    glm::mat4 rotation;
    glm::mat4 translation;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 perspective;
    glm::mat4 ortho;

    bool isPerspective = true;

    //Main window
    GLFWwindow* gWindow = nullptr;

    //Shader program
    GLuint gProgramId;

    //Cylinder quadric object
    GLUquadricObj* cyl;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];         // Handle for the vertex buffer object
        GLuint nIndices;    // Number of indices of the mesh
    };

    //Store coordinates for points
    struct GLCoord {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    };

    //Declare each mesh
    GLMesh basilMesh;
    GLMesh cayenneMesh;
    GLMesh pepperMesh;
    GLMesh mugMesh;
    GLMesh padMesh;
    GLMesh knitMesh;

    //VAOs and VBOs for each mesh
    unsigned int VBOknit, VBObasil, VBOcayenne, VBOpepper, VBOmug, VBOpad;
    unsigned int VAOknit, VAOcayenne, VAOpepper, VAOmug, VAOpad, VAObasil;

    //Camera variables
    glm::vec3 gCameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 gCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 gCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 gCameraRight = glm::normalize(glm::cross(gCameraFront, gCameraUp));

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    GLfloat halfScreenWidth = SCREEN_WIDTH / 2;
    GLfloat halfScreenHeight = SCREEN_HEIGHT / 2;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void generateTextures();
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreatePlaneMesh(GLMesh& mesh, GLCoord topRight, GLCoord topLeft, GLCoord bottomLeft, GLCoord bottomRight);
void UCreateCubeMesh(GLMesh& mesh, GLCoord topRight, GLCoord topLeft, GLCoord bottomLeft, GLCoord bottomRight, GLfloat depth);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1

out vec4 vertexColor; // variable to transfer color data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor);
}
);


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //Coordinates for BASIL
    struct GLCoord topRight = { 2.0f, 1.3f, 0.7f };
    struct GLCoord topLeft = { 1.0f, 1.3f, 0.2f };
    struct GLCoord bottomLeft = { 1.0f, -0.5f, 0.2f };
    struct GLCoord bottomRight = { 2.0f, -0.5f, 0.7f };
    // Create the mesh
    UCreateCubeMesh(basilMesh, topRight, topLeft, bottomRight, bottomLeft, 1.0f); // Calls the function to create the Vertex Buffer Object
    //Coordinates for CAYENNE
    topRight = { -1.0f, 1.3f, 0.2f };
    topLeft = { -2.0f, 1.3f, 0.7f };
    bottomLeft = { -2.0f, -0.5f, 0.7f };
    bottomRight = { -1.0f, -0.5f, 0.2f };
    UCreateCubeMesh(cayenneMesh, topRight, topLeft, bottomRight, bottomLeft, 1.0f); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;


    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        //set projections for perspective and ortho
        perspective = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        ortho = glm::ortho(-2.0f, +2.0f, -2.0f, +2.0f, 0.1f, 100.0f);
       
        // per frame timing
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
        // Model matrix: transformations are applied right-to-left order
        model = translation * rotation;

        view = gCamera.GetViewMatrix();

        // Retrieves and passes transform matrices to the Shader program
        GLint modelLoc = glGetUniformLocation(gProgramId, "model");
        GLint viewLoc = glGetUniformLocation(gProgramId, "view");
        GLint projLoc = glGetUniformLocation(gProgramId, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


        
        // input
        // -----
        UProcessInput(gWindow);
        if (isPerspective) {
            projection = perspective;
        }
        else {
			projection = ortho;
		}

        // Render this frame
        URender();

        glfwPollEvents();
    }
    UDestroyShaderProgram(gProgramId);

    // Release mesh data. Who knows what will happen if you keep it?
    UDestroyMesh(basilMesh);
    UDestroyMesh(cayenneMesh);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Function called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Set the shader to be used
    glUseProgram(gProgramId);

    //BASIL
    glBindVertexArray(basilMesh.vao);
    glDrawElements(GL_TRIANGLES, basilMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);
    //CAYENNE
    glBindVertexArray(cayenneMesh.vao);
    glDrawElements(GL_TRIANGLES, cayenneMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);



    //Swap buffers every frame. One is being transformed while the other is displayed
    glfwSwapBuffers(gWindow);
}



void UCreateCubeMesh(GLMesh& mesh, GLCoord topRight, GLCoord topLeft, GLCoord bottomLeft, GLCoord bottomRight, GLfloat depth)
{
    // Vertex data
    GLfloat verts[] = {
        //Front Face
        topRight.x,  topRight.y, topRight.z,                     1.0f, 0.0f, 0.0f, 0.8f, // Top Right Vertex 0
        bottomLeft.x, bottomLeft.y, bottomLeft.z,             0.0f, 1.0f, 0.0f, 0.8f, // Bottom Left Vertex 1
        bottomRight.x, bottomRight.y, bottomRight.z,                0.0f, 0.0f, 1.0f, 0.8f, // Bottom Right Vertex 2
        topLeft.x,  topLeft.y, topLeft.z,                        1.0f, 0.0f, 1.0f, 0.8f, // Top Left Vertex 3

        //Back Face                                                                
        topRight.x,  topRight.y, topRight.z - depth,             1.0f, 0.0f, 0.0f, 0.8f, // Top Right Vertex 4
        bottomLeft.x, bottomLeft.y, bottomLeft.z - depth,     0.0f, 1.0f, 0.0f, 0.8f, // Bottom Left Vertex 5
        bottomRight.x, bottomRight.y, bottomRight.z - depth,        0.0f, 0.0f, 1.0f, 0.8f, // Bottom Right Vertex 6
        topLeft.x,  topLeft.y, topLeft.z - depth,                1.0f, 0.0f, 1.0f, 0.8f  // Top Left Vertex 7
    };

    GLushort indices[] = {
        0, 1, 3,  // Triangle 1 (front)
        3, 2, 1,   // Triangle 2 (front)
        4, 5, 7,  // Triangle 3 (back)
        7, 6, 5,  // Triangle 4 (back)
        0, 4, 7, //Triangle 5 (top)
        7, 3, 0,  //Triangle 6 (top)
        1, 2, 5, //Triangle 7 (bottom)
        5, 6, 2, //Triangle 8 (bottom)
        0, 1, 5, //Triangle 9 (right)
        0, 4, 5, //Triangle 10 (right)
        3, 2, 6, //Triangle 11 (left)
        3, 7, 6 //Triangle 12 (left)
    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create buffers for vertex data and index data
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU
    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}


// Implements the UCreateShaders function
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

    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
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

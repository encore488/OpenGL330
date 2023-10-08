#include <iostream>
#include <cstdlib>
//OpenGL stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
#define PI 3.14159265359

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

    //Shader program(s?)
    GLuint gProgramId;
    GLuint gLampProgramId;

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
    GLMesh basilLidMesh;
    GLMesh cayenneMesh;
    GLMesh cayenneLidMesh;
    GLMesh pepperMesh;
    GLMesh mugMesh;
    GLMesh padMesh;
    GLMesh knitMesh;
    GLMesh tableMesh;

    // Texture id
    GLuint gTextureIdHappy;
    GLuint gTextureIdHat;
    bool gIsHatOn = true;

    //VAOs and VBOs for each mesh
    unsigned int VBOknit, VBObasil, VBOcayenne, VBObasilLid, VBOpepper, VBOmug, VBOpad, VBOtable;
    unsigned int VAOknit, VAOcayenne, VAOpepper, VAOmug, VAObasilLid, VAOpad, VAObasil, VAOtable;

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
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreatePlaneMesh(GLMesh& mesh, GLCoord topRight, GLCoord topLeft, GLCoord bottomLeft, GLCoord bottomRight);
void UCreateCubeMesh(GLMesh& mesh, GLCoord topRight, GLCoord topLeft, GLCoord bottomLeft, GLCoord bottomRight, GLfloat depth);
void UCreateCylinderMesh(GLMesh& mesh, GLfloat radius, GLCoord base, GLfloat depth);
void UCreatePyramidMesh(GLMesh& mesh, GLCoord top, GLfloat height, GLfloat width);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1
layout(location = 2) in vec2 textureCoordinate;

out vec4 vertexColor; // variable to transfer color data to the fragment shader
out vec2 vertexTextureCoordinate;

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

uniform sampler2D uTextureBase;
uniform sampler2D uTextureExtra;
uniform bool multipleTextures;

void main()
{
    fragmentColor = texture(uTextureBase, vertexTextureCoordinate);
    if (multipleTextures)
    {
        vec4 extraTexture = texture(uTextureExtra, vertexTextureCoordinate);
        if (extraTexture.a != 0.0)
            fragmentColor = extraTexture;
    }
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //Coordinates for BASIL
    struct GLCoord topRight = { 2.0f, -0.5f, 0.7f };
    struct GLCoord topLeft = { 1.0f, -0.5f, 0.2f };
    struct GLCoord bottomLeft = { 1.0f, -2.99f, 0.2f };
    struct GLCoord bottomRight = { 2.0f, -2.99f, 0.7f };
    struct GLCoord lidCenterBase = { 1.5f, -0.51f, -0.05f };
    // Create the meshs
    UCreatePlaneMesh(tableMesh, { 25.0f, -3.0f, 25.0f }, { -25.0f, -3.0f, 25.0f }, { -25.0f, -3.0f, -25.0f }, { 25.0f, -3.0f, -25.0f });
    UCreatePyramidMesh(knitMesh, { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f);
    UCreateCubeMesh(basilMesh, topRight, topLeft, bottomRight, bottomLeft, 1.0f);
    UCreateCylinderMesh(basilLidMesh, 0.7f, lidCenterBase, 0.4f);
      //Coordinates for CAYENNE
    topRight = { -2.0f, -0.5f, 0.2f };
    topLeft = { -3.0f, -0.5f, 0.7f };
    bottomLeft = { -3.0f, -2.99f, 0.7f };
    bottomRight = { -2.0f, -2.99f, 0.2f };
    UCreateCubeMesh(cayenneMesh, topRight, topLeft, bottomRight, bottomLeft, 1.0f); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "C://Users//encor//OneDrive//Pictures//theCounter.jpg";
    if (!UCreateTexture(texFilename, gTextureIdHappy))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "C://Users//encor//OneDrive//Pictures//theStones.jpg";
    if (!UCreateTexture(texFilename, gTextureIdHat))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureBase"), 0);
    // We set the texture as texture unit 1
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureExtra"), 1);



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
    // Release texture
    UDestroyTexture(gTextureIdHappy);
    UDestroyTexture(gTextureIdHat);
    UDestroyShaderProgram(gProgramId);

    // Release mesh data. Who knows what will happen if you keep it?
    UDestroyMesh(basilMesh);
    UDestroyMesh(cayenneMesh);
    UDestroyMesh(basilLidMesh);
    UDestroyMesh(padMesh);
    UDestroyMesh(knitMesh);
    UDestroyMesh(mugMesh);
    UDestroyMesh(pepperMesh);
    UDestroyMesh(tableMesh);

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
    static const float cameraSpeed = 1.8f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !gIsHatOn)
        gIsHatOn = true;
    else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && gIsHatOn)
        gIsHatOn = false;


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        isPerspective = false;
    //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        //shootLaser();
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

    // 1. Scales the shape down by half of its original size in all 3 dimensions
    glm::mat4 scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

    // 2. Rotates shape by 45 degrees on the z axis
    glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    // 3. Translates by 0.5 in the y axis
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.5f, 0.0f));

    // Transformations are applied right-to-left order
    //glm::mat4 transformation = translation * rotation * scale;
    glm::mat4 transformation(1.0f);

    //Set the shader to be used
    glUseProgram(gProgramId);


    // Sends transform information to the Vertex shader
    GLuint transformLocation = glGetUniformLocation(gProgramId, "shaderTransform");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transformation));

    GLuint multipleTexturesLoc = glGetUniformLocation(gProgramId, "multipleTextures");
    glUniform1i(multipleTexturesLoc, gIsHatOn);



    //BASIL
    glBindVertexArray(basilMesh.vao);

    
    glDrawElements(GL_TRIANGLES, basilMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);
    glBindVertexArray(basilLidMesh.vao);
    glDrawElements(GL_TRIANGLES, basilLidMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    //CAYENNE
    glBindVertexArray(cayenneMesh.vao);
    glDrawElements(GL_TRIANGLES, cayenneMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    //KNIT
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdHappy);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureIdHat);
    //Draw the stuff
    glBindVertexArray(knitMesh.vao);
    glDrawElements(GL_TRIANGLES, knitMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);
    
    //TABLE
    glBindVertexArray(tableMesh.vao);
    glDrawElements(GL_TRIANGLES, tableMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
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

void UCreatePyramidMesh(GLMesh& mesh, GLCoord top, GLfloat height, GLfloat width)
{
    // Vertex data
    GLfloat verts[] = {
        // Coordinates                                 Color                    Textures
        //Front Face
        top.x,  top.y, top.z,                          1.0f, 0.0f, 0.0f, 0.3f,  0.5f, 1.0f,  // Top Point Vertex 0
        top.x - width, top.y - height, top.z - width,  0.0f, 1.0f, 0.0f, 0.8f,  0.0f, 0.0f,  // Front Bottom Left Vertex 1
        top.x + width, top.y - height, top.z - width,  0.0f, 0.0f, 1.0f, 0.9f,  1.0f, 0.0f,  // Front Bottom Right Vertex 2 
        //Right Face
        top.x + width, top.y - height, top.z + width,  0.0f, 1.0f, 0.0f, 0.8f,  0.0f, 0.0f,  // Back Right Vertex 3
        top.x + width, top.y - height, top.z - width,  0.0f, 0.0f, 1.0f, 0.9f,  1.0f, 0.0f,  // Front Bottom Right Vertex 4
        //Left Face
        top.x - width, top.y - height, top.z - width,  0.0f, 1.0f, 0.0f, 0.8f,  1.0f, 0.0f,  // Front Bottom Left Vertex 5
        top.x - width, top.y - height, top.z + width,  1.0f, 0.0f, 1.0f, 0.2f,  0.0f, 0.0f,  // Back Left Vertex 6
        //Back Face
       top.x + width, top.y - height, top.z + width,  0.0f, 1.0f, 0.0f, 0.8f,  1.0f, 0.0f,  // Back Right Vertex 7
       top.x - width, top.y - height, top.z + width,  1.0f, 0.0f, 1.0f, 0.2f,  0.0f, 0.0f,  // Back Left Vertex 8
       //Bottom
       top.x + width, top.y - height, top.z + width,  0.0f, 1.0f, 0.0f, 0.8f,  1.0f, 1.0f,  // Back Right Vertex 9

    };

    GLushort indices[] = {
        0, 1, 2,  // Triangle 1 (front)
        0, 3, 4,   // Triangle 2 (right)
        0, 5, 6,  // Triangle 3 (left)
        0, 7, 8,  // Triangle 4 (back)
        1, 2, 9, //Triangle 5 (bottom)
        9, 6, 5  //Triangle 6 (bottom)
    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;

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
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        //        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // Set the texture wrapping parameters.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set texture filtering parameters.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture.

        return true;
    }

    // Error loading the image
    return false;
}

void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


void UCreateCylinderMesh(GLMesh& mesh, GLfloat radius, GLCoord base, GLfloat height) {
    const int numPoints = 60;
    GLfloat angleIncrement = 2 * PI / numPoints;
    std::vector<GLfloat> verts;
    std::vector<GLushort> indices;

    //Let the first indice be the center of the bottom circle, then the second should be center of the top circle
    verts.push_back(base.x);
    verts.push_back(base.y);
    verts.push_back(base.z);
    verts.push_back(base.x);
    verts.push_back(base.y + height);
    verts.push_back(base.z);

    // Create vertices for the sides
    for (int i = 0; i < numPoints; ++i) {
        GLfloat x = base.x + radius * cos(i * angleIncrement);
        GLfloat z = base.z + radius * sin(i * angleIncrement);
        //Bottom side 1
        verts.push_back(x);
        verts.push_back(base.y);
        verts.push_back(z);
        //Top Side 1
        verts.push_back(x);
        verts.push_back(base.y + height);
        verts.push_back(z);
    }

    // Connect the vertices to form the cylinder, one slice at a time
    for (int i = 0; i < (numPoints - 1); ++i) {
        int bottomVert = 2 * i + 2;
        int topVert = 2 * i + 3;
        int nextBottomVert = bottomVert + 2;
        int nextTopVert = topVert + 2;

        //Bottom Pie Slice
        indices.push_back(0);
        indices.push_back(bottomVert);
        indices.push_back(nextBottomVert);

        //Top Pie Slice
        indices.push_back(1);
        indices.push_back(nextTopVert);
        indices.push_back(topVert);

        //Side 1
        indices.push_back(bottomVert);
        indices.push_back(nextBottomVert);
        indices.push_back(topVert);

        //Side 2
        indices.push_back(nextBottomVert);
        indices.push_back(topVert);
        indices.push_back(nextTopVert);
    }

    // Generate VAO and VBOs
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.vbos[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

    // Vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    mesh.nIndices = indices.size();
    glBindVertexArray(0);
}


void UCreatePlaneMesh(GLMesh& mesh, GLCoord topRight, GLCoord topLeft, GLCoord bottomLeft, GLCoord bottomRight) {
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    GLfloat verts[] = {
        //Front Face
        topRight.x,  topRight.y, topRight.z,                     0.2f, 0.1f, 0.1f, 1.0f, // Top Right Vertex 0
        bottomLeft.x, bottomLeft.y, bottomLeft.z,             0.1f, 0.2f, 0.2f, 1.0f, // Bottom Left Vertex 1
        bottomRight.x, bottomRight.y, bottomRight.z,                0.2f, 0.2f, 0.3f, 1.0f, // Bottom Right Vertex 2
        topLeft.x,  topLeft.y, topLeft.z,                        0.2f, 0.3f, 0.2f, 1.0f, // Top Left Vertex 3
    };

    GLushort indices[] = {
        0, 2, 3,  // Triangle 1
        3, 1, 2   // Triangle 2 
    };

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

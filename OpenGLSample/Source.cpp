#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

using namespace std;

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif


namespace
{
    const char* const WINDOW_TITLE = "Window of Justice";
    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 800;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];         // Handle for the vertex buffer object
        GLuint nVertices;
        GLuint nIndices;
    };

    //Store coordinates for points
    struct GLCoord {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    };

    #define PI 3.14159265359
    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    
    ////////////////// Create basil mesh and assign all necessary values   //////////////////
    GLMesh basilMesh;
    GLuint basilTextureId;
    GLint gTexWrapMode = GL_REPEAT;
    // Position and scale
    glm::vec3 gBasilPosition(-3.0f, -0.2f, 0.0f);
    glm::vec3 gBasilScale(2.0f);
    glm::vec2 gUVScale(1.0f, 1.0f);
    GLMesh basilLidMesh;
    GLuint basilLidTextureId;
    glm::vec3 gBasilLidPosition(-3.0f, 2.2f, 0.0f);
    //////////////////
    
    // Create Pyramid mesh and assign all necessary values
    GLMesh pyrMesh;
    GLuint pyrTextureId;
    // Pyramid position and scale
    glm::vec3 gPyramidPosition(3.0f, -0.2f, 0.0f);
    glm::vec3 gPyramidScale(2.0f);
    glm::vec2 gPyramidUVScale(6.0f, 6.0f);

    // Create Mug mesh and assign all necessary values
    GLMesh mugMesh;
    glm::vec3 gMugPosition(3.0f, -0.2f, 5.0f);
    glm::vec3 gMugScale(2.0f);
    glm::vec2 gMugUVScale(1.0f, 1.0f);


    // Cayenne jar
    GLMesh cayenneMesh;
    GLuint cayenneTextureId;
    glm::vec3 gCayennePosition(-3.0f, -0.2f, 3.0f);
    glm::vec3 gCayenneScale(2.0f);
    glm::vec2 gCayenneUVScale(1.0f, 1.0f);

    // Hot pad
    GLMesh padMesh;
    GLuint padTextureId;
    glm::vec3 gPadPosition(0.0f, -0.18f, 4.0f);
    glm::vec3 gPadScale(2.0f);


    GLMesh cayenneLidMesh;
    GLuint cayenneLidTextureId;
    glm::vec3 gCayenneLidPosition(-3.0f, 2.2f, 3.0f);

    // Table
    GLMesh tableMesh;
    GLuint tableTextureId;
    glm::vec3 gTablePosition(-3.0f, -0.2f, 3.0f);
    glm::vec3 gTableScale(2.0f);
    glm::vec2 gTableUVScale(6.0f, 6.0f);

    // Shader programs
    GLuint gCubeProgramId;
    GLuint gLampProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    

    // Cube and lights color
    glm::vec3 gObjectColor(1.0f, 0.7f, 0.7f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gFillLightColor(1.0f, 0.0f, 1.0f);

    // Lights position and scale
    glm::vec3 gLightPosition(1.5f, 0.8f, 2.0f);
    glm::vec3 gLightScale(0.4f);
    glm::vec3 gFillLightPosition(8.5f, 0.5f, 1.0f);
    glm::vec3 gFillLightScale(0.1f);

    // Lamp animation
    bool gIsLampOrbiting = true;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UCreatePlaneMesh(GLMesh& mesh, GLCoord bl, GLCoord br, GLCoord fl, GLCoord fr);
void UCreatePyramidMesh(GLMesh& mesh, GLCoord top, GLfloat height, GLfloat width);
void UCreateCubeMesh(GLMesh& mesh, GLCoord top, GLfloat height, GLfloat width);
void UCreateCylinderMesh(GLMesh& mesh, GLfloat radius, GLfloat height, GLCoord base);
void UCreateCircleMesh(GLMesh& mesh, GLfloat radius, GLCoord center);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


////////////////////////////////////////////////Shaders//////////////////////////////////////////////
/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.3f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(0.8f, 1.0f, 1.0f, 1.0f); // Set color and alpha of lamps
}
);
/////////////////////////////////// ^^ Shaders ^^ /////////////////////////////////////////////////


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

    // Create the mesh
    UCreateCubeMesh(basilMesh, { -3.0f, 2.0f, 0.0f }, 2.0f, 1.0f);
    UCreatePyramidMesh(pyrMesh, { 3.0f, 1.0f, 3.0f }, 1.0f, 1.0f);
    UCreateCubeMesh(cayenneMesh, { -3.0f, 2.0f, 3.0f }, 2.0f, 1.0f);
    UCreatePlaneMesh(tableMesh, { -13.0f, 0.0f, -13.0f }, { 13.0f, 0.0f, -13.0f }, { -13.0f, 0.0f, 13.0f }, { 13.0f, 0.0f, 13.0f });
    UCreateCylinderMesh(basilLidMesh, 0.6f, 0.3f, { -3.0f, 2.01f, 0.0f });
    UCreateCylinderMesh(cayenneLidMesh, 0.6f, 0.3f, { -3.0f, 2.01f, 3.0f });
    UCreateCylinderMesh(mugMesh, 0.7f, 1.4f, { 3.0f, -0.2f, 5.0f });
    UCreateCircleMesh(padMesh, 1.0f, { 0.0f, 0.01f, -4.0f });

    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Load textures
    const char* texFilename = "C://Users//encor//Downloads//basilLabel.jpeg";
    if (!UCreateTexture(texFilename, basilTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    const char* pyrTexFilename = "C://Users//encor//OneDrive//Pictures//theStones.jpg";
if (!UCreateTexture(pyrTexFilename, pyrTextureId))
	{
		cout << "Failed to load texture " << pyrTexFilename << endl;
		return EXIT_FAILURE;
	}
	const char* cayenneTexFilename = "C://Users//encor//OneDrive//Pictures//cayenneLabel.jpg";
    if (!UCreateTexture(cayenneTexFilename, cayenneTextureId))
    {
		cout << "Failed to load texture " << cayenneTexFilename << endl;
		return EXIT_FAILURE;
	}
    const char* tableTexFilename = "C://Users//encor//Downloads//table.jpeg";
    if (!UCreateTexture(tableTexFilename, tableTextureId))
    {
		cout << "Failed to load texture " << tableTexFilename << endl;
		return EXIT_FAILURE;
	}
    const char* lidTexFilename = "C://Users//encor//Downloads//black.jpeg";
    if (!UCreateTexture(lidTexFilename, basilLidTextureId))
        {
        cout << "Failed to load texture " << lidTexFilename << endl;
			return EXIT_FAILURE;
		}
    const char* padTexFilename = "C://Users//encor//Downloads//cork.jpeg";
    if (!UCreateTexture(padTexFilename, padTextureId))
    {
		cout << "Failed to load texture " << padTexFilename << endl;
		return EXIT_FAILURE;
	}
   


    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gCubeProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data. Who knows what will happen if we keep it?
    UDestroyMesh(basilMesh);
    UDestroyMesh(pyrMesh);
	UDestroyMesh(cayenneMesh);
    UDestroyMesh(tableMesh);
    UDestroyMesh(basilLidMesh);
    UDestroyMesh(mugMesh);
    UDestroyMesh(cayenneLidMesh);
    UDestroyMesh(padMesh);

    // Release texture
    UDestroyTexture(basilTextureId);
    UDestroyTexture(pyrTextureId);
    UDestroyTexture(cayenneTextureId);
    UDestroyTexture(tableTextureId);
    UDestroyTexture(basilLidTextureId);
    UDestroyTexture(padTextureId);

    // Release shader programs
    UDestroyShaderProgram(gCubeProgramId);
    UDestroyShaderProgram(gLampProgramId);

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

    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 1.8f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


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
   //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        //shootLaser();


    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;

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




void URender()
{
    // Lamp orbits around the origin
    const float angularVelocity = glm::radians(25.0f);
    if (gIsLampOrbiting)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 1.0f)) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
    }

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the VAO (used by cubes and lamps)
    glBindVertexArray(basilMesh.vao);

    // Basil Creation!
    //----------------
    // Set the shader to be used
    glUseProgram(gCubeProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gBasilPosition) * glm::scale(gBasilScale);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint fillLightColorLoc = glGetUniformLocation(gCubeProgramId, "fillLightColor");
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint fillLightPositionLoc = glGetUniformLocation(gCubeProgramId, "fillLightPos");
    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");


    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(fillLightColorLoc, gFillLightColor.r, gFillLightColor.g, gFillLightColor.b);
    glUniform3f(fillLightPositionLoc, gFillLightPosition.x, gFillLightPosition.y, gFillLightPosition.z);

    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, basilTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, basilMesh.nVertices);

    // LAMP: draw key lamp
    //----------------
    glUseProgram(gLampProgramId);

    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, basilMesh.nVertices);


    // LAMP: draw fill lamp

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gFillLightPosition) * glm::scale(gFillLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, basilMesh.nVertices);

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);


    // Pyramid Creation!
    glBindVertexArray(pyrMesh.vao);

    // Set the shader to be used
    glUseProgram(gCubeProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model2 = glm::translate(gPyramidPosition) * glm::scale(gPyramidScale);

    // camera/view transformation
    glm::mat4 view2 = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection2 = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc2 = glGetUniformLocation(gCubeProgramId, "model2");
    GLint viewLoc2 = glGetUniformLocation(gCubeProgramId, "view2");
    GLint projLoc2 = glGetUniformLocation(gCubeProgramId, "projection2");

    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model2));
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view2));
    glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(projection2));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc2 = glGetUniformLocation(gCubeProgramId, "objectColor2");
    GLint lightColorLoc2 = glGetUniformLocation(gCubeProgramId, "lightColor2");
    GLint lightPositionLoc2 = glGetUniformLocation(gCubeProgramId, "lightPos2");
    GLint viewPositionLoc2 = glGetUniformLocation(gCubeProgramId, "viewPosition2");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc2, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc2, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc2, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(viewPositionLoc2, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gPyramidUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pyrTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, pyrMesh.nVertices);
    // ^^ Pyramid Creation ^^

    // Cayenne Creation!
    glBindVertexArray(cayenneMesh.vao);

    // Set the shader to be used
    glUseProgram(gCubeProgramId);


    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model2));
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view2));
    glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(projection2));


    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc2, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc2, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc2, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(viewPositionLoc2, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gCayenneUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cayenneTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, cayenneMesh.nVertices);
    // ^^ Cayenne Creation ^^

        // Cayenne Lid Creation!
    glBindVertexArray(cayenneLidMesh.vao);

    // Set the shader to be used
    glUseProgram(gCubeProgramId);


    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model2));
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view2));
    glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(projection2));


    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc2, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc2, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc2, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(viewPositionLoc2, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gCayenneUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tableTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, cayenneLidMesh.nVertices);
    // ^^ Cayenne Lid Creation ^^

            // Basil Lid Creation!
    glBindVertexArray(basilLidMesh.vao);

    // Set the shader to be used
    glUseProgram(gCubeProgramId);


    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model2));
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view2));
    glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(projection2));


    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc2, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc2, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc2, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(viewPositionLoc2, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tableTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, basilLidMesh.nVertices);
    // ^^ Basil Lid Creation ^^


                // Mug Creation!
    glBindVertexArray(mugMesh.vao);

    // Set the shader to be used
    glUseProgram(gCubeProgramId);
    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model2));
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view2));
    glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(projection2));

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc2, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc2, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc2, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(viewPositionLoc2, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, basilLidTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, mugMesh.nVertices);
    // ^^ Mug Creation ^^

    // Table Creation!
    glBindVertexArray(tableMesh.vao);

    // Set the shader to be used
    glUseProgram(gCubeProgramId);


    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model2));
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view2));
    glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(projection2));


    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc2, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc2, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc2, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(viewPositionLoc2, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gTableUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tableTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, tableMesh.nVertices);
    // ^^ Table Creation ^^

    // Pad Creation!
    glBindVertexArray(padMesh.vao);
    // Set the shader to be used
    glUseProgram(gCubeProgramId);

    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model2));
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view2));
    glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(projection2));

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc2, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc2, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc2, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(viewPositionLoc2, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gCayenneUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, padTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, padMesh.nVertices);
    // ^^ Hot pad Creation ^^


    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}




void UCreateCubeMesh(GLMesh& mesh, GLCoord top, GLfloat height, GLfloat width)
{
    // Position and Texture data
    GLfloat verts[] = {
        // ------------------------------------------------------
        //Back Face                                          //Negative Z Normal  Texture Coords.
       top.x - (width / 2), top.y - height, top.z - (width/2),  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        top.x + (width / 2), top.y - height, top.z - (width / 2),  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        top.x + (width / 2),  top.y, top.z - (width / 2),  0.0f,  0.0f, -1.0f,  0.5f, 1.0f,
        top.x + (width / 2),  top.y, top.z - (width / 2),  0.0f,  0.0f, -1.0f,  0.5f, 1.0f,
       top.x - (width / 2),  top.y, top.z - (width / 2),  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       top.x - (width / 2), top.y - height, top.z - (width / 2),  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

       //Front Face                                              //Positive Z Normal
      top.x - (width / 2), top.y - height,  top.z + (width / 2),  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       top.x + (width / 2), top.y - height,  top.z + (width / 2),  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       top.x + (width / 2),  top.y,  top.z + (width / 2),  0.0f,  0.0f,  1.0f,  0.5f, 1.0f,
       top.x + (width / 2),  top.y,  top.z + (width / 2),  0.0f,  0.0f,  1.0f,  0.5f, 1.0f,
      top.x - (width / 2),  top.y,  top.z + (width / 2),  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
      top.x - (width / 2), top.y - height,  top.z + (width / 2),  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

      //Left Face                                                //Negative X Normal
     top.x - (width / 2),  top.y,  top.z + (width / 2), -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     top.x - (width / 2),  top.y, top.z - (width / 2), -1.0f,  0.0f,  0.0f,  0.5f, 1.0f,
     top.x - (width / 2), top.y - height, top.z - (width / 2), -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     top.x - (width / 2), top.y - height, top.z - (width / 2), -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     top.x - (width / 2), top.y - height,  top.z + (width / 2), -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     top.x - (width / 2),  top.y,  top.z + (width / 2), -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

     //Right Face                                              //Positive X Normal
     top.x + (width / 2),  top.y,  top.z + (width / 2),  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     top.x + (width / 2),  top.y, top.z - (width / 2),  1.0f,  0.0f,  0.0f,  0.5f, 1.0f,
     top.x + (width / 2), top.y - height, top.z - (width / 2),  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  //Bottom Front?
     top.x + (width / 2), top.y - height, top.z - (width / 2),  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     top.x + (width / 2), top.y - height,  top.z + (width / 2),  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     top.x + (width / 2),  top.y,  top.z + (width / 2),  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

     //Bottom Face                                             //Negative Y Normal
    top.x - (width / 2), top.y - height, top.z - (width / 2),  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     top.x + (width / 2), top.y - height, top.z - (width / 2),  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     top.x + (width / 2), top.y - height,  top.z + (width / 2),  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     top.x + (width / 2), top.y - height,  top.z + (width / 2),  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    top.x - (width / 2), top.y - height,  top.z + (width / 2),  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    top.x - (width / 2), top.y - height, top.z - (width / 2),  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //Top Face                                                 //Positive Y Normal
   top.x - (width / 2),  top.y, top.z - (width / 2),  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    top.x + (width / 2),  top.y, top.z - (width / 2),  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    top.x + (width / 2),  top.y,  top.z + (width / 2),  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    top.x + (width / 2),  top.y,  top.z + (width / 2),  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   top.x - (width / 2),  top.y,  top.z + (width / 2),  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   top.x - (width / 2),  top.y, top.z - (width / 2),  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create buffer for vertex data
    glGenBuffers(1, &mesh.vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is all the data required to plot one vertex
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void UCreatePyramidMesh(GLMesh& mesh, GLCoord top, GLfloat height, GLfloat width)
{
    // Position and Texture data
    GLfloat verts[] = {

        //Back Face                                              //Negative Z Normal  Texture Coords.
       top.x - (width / 2), top.y - height, top.z - (width / 2),  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        top.x + (width / 2), top.y - height, top .z - (width / 2),  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        top.x,  top.y, top.z,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,

       //Front Face                                             //Positive Z Normal
      top.x - (width / 2), top.y - height,  top.z + (width / 2),  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       top.x + (width / 2), top.y - height,  top.z + (width / 2),  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       top.x,  top.y,  top.z,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,

      //Left Face                          //Negative X Normal
     top.x,  top.y,  top.z, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     top.x - (width / 2),  top.y - height, top.z + (width / 2), -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     top.x - (width / 2), top.y - height, top.z - (width / 2), -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     
     //Right Face                                           //Positive X Normal
     top.x + (width / 2), top.y - height, top.z - (width / 2),  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     top.x + (width / 2), top.y - height,  top.z + (width / 2),  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     top.x,  top.y,  top.z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Bottom Face                                          //Negative Y Normal
    top.x - (width / 2), top.y - height, top.z - (width / 2),  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     top.x + (width / 2), top.y - height, top.z - (width / 2),  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     top.x + (width / 2), top.y - height,  top.z + (width / 2),  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     top.x + (width / 2), top.y - height,  top.z + (width / 2),  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    top.x - (width / 2), top.y - height,  top.z + (width / 2),  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    top.x - (width / 2), top.y - height, top.z - (width / 2),  0.0f, -1.0f,  0.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create buffer for the vertex data
    glGenBuffers(1, &mesh.vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreatePlaneMesh(GLMesh& mesh, GLCoord bl, GLCoord br, GLCoord fl, GLCoord fr)
{
    // Position and Vertex data
    GLfloat verts[] = {
       //Coordinates       // Normals         //Texture Coords.
       bl.x, bl.y, bl.z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,    //Back Left
       br.x, br.y, br.z,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,    //Back Right
       fr.x, fr.y,  fr.z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,	 //Front Right
       fr.x, fr.y,  fr.z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,	 //Front Right
       fl.x, fl.y,  fl.z,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,	 //Front Left
       bl.x, bl.y, bl.z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f	 //Back Left
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateCircleMesh(GLMesh& mesh, GLfloat radius, GLCoord center) {
    const GLint numSegments = 60;    // The number of triangles used to draw the circle
    const GLint floatsPerVertex = 3;

    mesh.nVertices = numSegments * 3; // Each segment creates a triangle (3 vertices)

    GLfloat* verts = new GLfloat[mesh.nVertices * floatsPerVertex];   // Create array to hold the vertex data

    // Calculate the angle in radians between segments
    GLfloat angleIncrement = (2.0f * PI) / static_cast<GLfloat>(numSegments);

    // Create vertices for the circle
    for (int i = 0; i < numSegments; ++i) {
        // Calculate the current angle and the next angle (for the next segment)
        GLfloat angle = static_cast<GLfloat>(i) * angleIncrement;
        GLfloat nextAngle = static_cast<GLfloat>(i + 1) * angleIncrement;
        // Find the x and y coordinates of the first vertex
        GLfloat x = center.x + radius * cos(angle);
        GLfloat z = center.z + radius * sin(angle);
        // Find the x and y coordinates of the second vertex
        GLfloat nextX = center.x + radius * cos(nextAngle);
        GLfloat nextZ = center.z + radius * sin(nextAngle);

        // Vertex positions, starting with the center of the circle
        verts[i * 9] = center.x;
        verts[(i * 9) + 1] = center.y;
        verts[(i * 9) + 2] = center.z;
        verts[(i * 9) + 3] = x;
        verts[(i * 9) + 4] = center.y;
        verts[(i * 9) + 5] = z;
        verts[(i * 9) + 6] = nextX;
        verts[(i * 9) + 7] = center.y;
        verts[(i * 9) + 8] = nextZ;
    }

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh.nVertices * floatsPerVertex * sizeof(GLfloat), verts, GL_STATIC_DRAW);

    GLint stride = sizeof(float) * floatsPerVertex;

    // Vertex positions attribute pointer
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
}


void UCreateCylinderMesh(GLMesh& mesh, GLfloat radius, GLfloat height, GLCoord center) {
    //Create a for loop that makes a top circle, bottom circle, and planes to connect them
    // Each iteration of for loop should make 1 pie slice from the top, one slice from the bottom,
    //  and 2 triangles to form the plane that connects them. Then increment the angle

    const GLint numSegments = 60;    // The number of triangles used to draw the circle
    const GLint floatsPerSegment = 36;  // 4 Triangles * 3 Vertices * 3 floats (x, y, z)
    const GLint floatsPerVertex = 3;    // 3 floats per vertex (x, y, z)

    mesh.nVertices = numSegments * 12; // Each segment creates 4 triangles with 3 vertices each

    GLfloat* verts = new GLfloat[numSegments * floatsPerSegment];   // Create array to hold the vertex data

    // Calculate the angle in radians between segments
    GLfloat angleIncrement = (2.0f * PI) / static_cast<GLfloat>(numSegments);

    // Create vertices for each of the 4 triangles in each segment
    for (int i = 0; i < numSegments; ++i) {
        // Calculate the current angle and the next angle (for the next segment)
        GLfloat angle = static_cast<GLfloat>(i) * angleIncrement;
        GLfloat nextAngle = static_cast<GLfloat>(i + 1) * angleIncrement;
        // Find the x and z coordinates of the first vertex of first triangle
        GLfloat x = center.x + radius * cos(angle);
        GLfloat z = center.z + radius * sin(angle);
        // Find the x and z coordinates of the second vertex
        GLfloat nextX = center.x + radius * cos(nextAngle);
        GLfloat nextZ = center.z + radius * sin(nextAngle);
        // Y value of top circle
        GLfloat topY = center.y + height;

        // Vertex positions, starting with the center of the bottom circle
        //Bottom slice
        verts[i * floatsPerSegment] = center.x;
        verts[(i * floatsPerSegment) + 1] = center.y;
        verts[(i * floatsPerSegment) + 2] = center.z;
        verts[(i * floatsPerSegment) + 3] = x;
        verts[(i * floatsPerSegment) + 4] = center.y;
        verts[(i * floatsPerSegment) + 5] = z;
        verts[(i * floatsPerSegment) + 6] = nextX;
        verts[(i * floatsPerSegment) + 7] = center.y;
        verts[(i * floatsPerSegment) + 8] = nextZ;
        // Top slice
        verts[(i * floatsPerSegment) + 9] = center.x;
        verts[(i * floatsPerSegment) + 10] = topY;
        verts[(i * floatsPerSegment) + 11] = center.z;
        verts[(i * floatsPerSegment) + 12] = x;
        verts[(i * floatsPerSegment) + 13] = topY;
        verts[(i * floatsPerSegment) + 14] = z;
        verts[(i * floatsPerSegment) + 15] = nextX;
        verts[(i * floatsPerSegment) + 16] = topY;
        verts[(i * floatsPerSegment) + 17] = nextZ;
        // Side Plane, starting with top two points, then bottom current point
        verts[(i * floatsPerSegment) + 18] = x;
        verts[(i * floatsPerSegment) + 19] = topY;
        verts[(i * floatsPerSegment) + 20] = z;
        verts[(i * floatsPerSegment) + 21] = nextX;
        verts[(i * floatsPerSegment) + 22] = topY;
        verts[(i * floatsPerSegment) + 23] = nextZ;
        verts[(i * floatsPerSegment) + 24] = x;
        verts[(i * floatsPerSegment) + 25] = center.y;
        verts[(i * floatsPerSegment) + 26] = z;
		// Bottom points, then next top point
        verts[(i * floatsPerSegment) + 27] = nextX;
        verts[(i * floatsPerSegment) + 28] = center.y;
        verts[(i * floatsPerSegment) + 29] = nextZ;
        verts[(i * floatsPerSegment) + 30] = x;
        verts[(i * floatsPerSegment) + 31] = center.y;
        verts[(i * floatsPerSegment) + 32] = z;
        verts[(i * floatsPerSegment) + 33] = nextX;
        verts[(i * floatsPerSegment) + 34] = topY;
        verts[(i * floatsPerSegment) + 35] = nextZ;
    }

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, numSegments * floatsPerSegment * sizeof(GLfloat), verts, GL_STATIC_DRAW);

    GLint stride = sizeof(float) * floatsPerVertex;

    // Vertex positions attribute pointer
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbos[0]);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gTexWrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gTexWrapMode);
        // set texture filtering parameters
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
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
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

    // Compile the vertex shader, and print compilation errors (if any)
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

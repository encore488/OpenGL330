#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct GLCoord {
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

class Cube {
public:
    Cube(float width, float height, const std::string& texturePath, const GLCoord& coordinates)
        : width(width), height(height), coordinates(coordinates) {
        // Load texture
        diffuseMap = loadTexture(texturePath.c_str());

        // Vertex data for a textured cube
        float vertices[] = {
            // Positions           // Texture Coordinates
            -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,

            // Add more vertices for the other faces if necessary
        };

        // Initialize VAO, VBO, and other OpenGL buffers
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Unbind VAO
        glBindVertexArray(0);
    }

    void Draw(Shader& shader) {
        shader.use();

        // Set model matrix with translation based on coordinates
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(coordinates.x, coordinates.y, coordinates.z));
        shader.setMat4("model", model);

        // Bind texture
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        // Bind VAO and draw the cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_QUADS, 0, 24); // Draw the cube as quads
        glBindVertexArray(0);
    }

    ~Cube() {
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &VBO);
        glDeleteTextures(1, &diffuseMap);
    }

private:
    float width, height;
    GLCoord coordinates;
    unsigned int cubeVAO, VBO, diffuseMap;

    unsigned int loadTexture(const char* path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrChannels;
        unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
};

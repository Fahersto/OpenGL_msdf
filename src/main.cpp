#include <memory>
#include <chrono>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "Renderer.hpp"
#include "FontAtlas.hpp"

Renderer renderer;
int8_t keys_[1024];
double scrollWheel_;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            keys_[key] = 3;
        }
        else if (action == GLFW_RELEASE)
        {
            keys_[key] = 0;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    scrollWheel_ = yoffset;
}

bool GetKeyDown(int key)
{
    return keys_[key] & 2;
}

bool GetKey(int key)
{
    return keys_[key] & 3;
}

void HandleCamera(double deltaTimeMilliSeconds)
{
    glm::vec2 currentCameraPosition = renderer.GetCameraPosition();
    glm::vec2 cameraChange = glm::vec2(0, 0);
    const float speed = 0.1f;

    if (GetKey(GLFW_KEY_LEFT) || GetKey(GLFW_KEY_A))
    {
        cameraChange -= glm::vec2(speed * deltaTimeMilliSeconds, 0);
    }

    if (GetKey(GLFW_KEY_RIGHT) || GetKey(GLFW_KEY_D))
    {
        cameraChange += glm::vec2(speed * deltaTimeMilliSeconds, 0);
    }

    if (GetKey(GLFW_KEY_UP) || GetKey(GLFW_KEY_W))
    {
        cameraChange += glm::vec2(0, speed * deltaTimeMilliSeconds);
    }

    if (GetKey(GLFW_KEY_DOWN) || GetKey(GLFW_KEY_S))
    {
        cameraChange -= glm::vec2(0, speed * deltaTimeMilliSeconds);
    }
    renderer.SetCameraPosition(currentCameraPosition + cameraChange);

    renderer.SetZoom(renderer.GetZoom() + scrollWheel_);
    if (renderer.GetZoom() < 0)
    {
        renderer.SetZoom(0);
    }
}

int main()
{
    //glm::vec2 windowSize = glm::vec2(2560, 1440);
    glm::vec2 windowSize = glm::vec2(1280, 720);
    GLFWwindow* window = renderer.CreateWindow("OpenGL - msdf - atlas", windowSize, glm::vec2(160, 90));
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    FontAtlas fontAtlas = FontAtlas("C:\\Windows\\Fonts\\arial.ttf");

    auto currentFrame = std::chrono::steady_clock::now();
    auto lastFrame = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(window))
    {
        // calculate delta time used for fps independent movement
        currentFrame = std::chrono::steady_clock::now();
        auto deltaTimeMilliSeconds = std::chrono::duration_cast<std::chrono::microseconds>(currentFrame - lastFrame).count() / 1000.f;

        // clear last frame
        glClearColor(.3f, .3f, .3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // handle user camera movement
        HandleCamera(deltaTimeMilliSeconds);
        
        // setup shader inputs
        renderer.BeginFrame();

        // draw the texture
        fontAtlas.DrawText(renderer, "Test");

        // draw frame
        glfwSwapBuffers(window);

        // reset input
        for (int i = 0; i < 1024; i++)
        {
            keys_[i] &= 1;
        }
        scrollWheel_ = 0;

        // poll new input
        glfwPollEvents();

        lastFrame = currentFrame;
    }
}
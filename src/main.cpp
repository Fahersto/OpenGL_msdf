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


std::string lorem = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr,  sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr,  sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr,  sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";


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
       //renderer.SetZoom(0);
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

    auto lastFpsDisplayTime = std::chrono::steady_clock::now();
    int fps = 0;
    int fpsDisplay = 0;
    while (!glfwWindowShouldClose(window))
    {
        // calculate delta time used for fps independent movement
        currentFrame = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(currentFrame - lastFpsDisplayTime).count() >= 1000000)
        {
            lastFpsDisplayTime = currentFrame;
            //printf("Fps: %d\n", fps);
            fpsDisplay = fps;
            fps = 0;
        }

        auto deltaTimeMilliSeconds = std::chrono::duration_cast<std::chrono::microseconds>(currentFrame - lastFrame).count() / 1000.f;

        // clear last frame
        glClearColor(.3f, .3f, .3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // handle user camera movement
        HandleCamera(deltaTimeMilliSeconds);
        
        // setup shader inputs
        renderer.BeginFrame();


        // disable depth testing to prevent issues with slightly  overlapping characters
        glDisable(GL_DEPTH_TEST);

        glm::vec4 color = glm::vec4(1, 0, 0, 0.3f);
        // draw the text
        //fontAtlas.DrawText("Hello  World!", glm::vec3(10, 10, 0), 20);
        fontAtlas.DrawText(renderer, std::to_string(fpsDisplay) + " fps", glm::vec3(0, 0, 0), 20, color);
        fontAtlas.DrawText(renderer, "ABCDEFG", glm::vec3(0, 10, 0), 10, color);
        fontAtlas.DrawText(renderer, "Hello World!", glm::vec3(0, 20, 0), 10, color);
        fontAtlas.DrawText(renderer, "123456789", glm::vec3(0, 30, 0), 10, color);
        fontAtlas.DrawText(renderer, "Hello\nWorld\n!", glm::vec3(0, 70, 0), 10, color);

        // enable depth testing
        glEnable(GL_DEPTH_TEST);
        
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
        fps++;
    }
}
#include "Renderer.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/ext.hpp"

#include "Shader.hpp"

glm::vec2 size = glm::vec2(40, 40);

Renderer::Renderer()
	: cameraPosition_(glm::vec2(0,0)), zoom_(1.0f)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* Renderer::CreateWindow(std::string name, glm::vec2 resolution, glm::vec2 worldUnits)
{
	screenSize_ = resolution;
	worldSize_ = worldUnits;

	window_ = glfwCreateWindow(resolution.x, resolution.y, name.c_str(), nullptr, nullptr);

	glfwMakeContextCurrent(window_);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("[Error] - Renderer - Failed to initialize GLAD\n");
		return nullptr;
	}

	glViewport(0, 0, resolution.x, resolution.y);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSwapInterval(0); //disable vsync
	//glfwSetWindowCloseCallback(window_, windowCloseCallback);

	// Setup Platform/Renderer backends
	const char* glsl_version = "#version 130";

	//TODO. The question here is: do I want the same things to be visible at any resolution? (probably). Therefore I woul need to hardcode the number.
	// 	   Or do I want to higher resolution to see more:
	//		projection_ = glm::ortho(0.0f, static_cast<float>(xResolution), 0.0f, static_cast<float>(yResolution), -1.f, 1.f);
	//The projection is used to normalize coordinates to the [-1, 1] range that OpenGL uses. This means that at any 16:9 ratio square will look like a square
	projection_ = glm::ortho(0.0f, worldUnits.x, 0.0f, worldUnits.y, -100.f, 100.f);


	shader_ = Shader::CompileFromFile("C:\\Users\\fahersto\\repos\\OpenGL_msdf\\src\\shader\\shader.vert", "C:\\Users\\fahersto\\repos\\OpenGL_msdf\\src\\shader\\shader.frag");
	shader_->Use();


	shader_->SetMatrix4("projection", projection_, true);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(20, 20, 0.0f));

	model = glm::translate(model, glm::vec3(size * 0.5f, 0.0f));

	model = glm::scale(model, glm::vec3(size, 1.0f));


	shader_->SetMatrix4("model", model);

	return window_;
}

void Renderer::BeginFrame()
{
	glm::mat4 camera(1.0f);
	camera = glm::scale(camera, glm::vec3(zoom_, zoom_, 1.0f));
	camera = glm::translate(camera, glm::vec3(cameraPosition_, 0.f));

	glm::vec2 sizeInPixels = this->EuToPixel(size) * this->GetZoom();
	shader_->SetVector2f("sizeInPixels", sizeInPixels);
	shader_->SetMatrix4("camera", camera);
}

glm::vec2 Renderer::GetCameraPosition()
{
	return cameraPosition_;
}

void Renderer::SetCameraPosition(glm::vec2 position)
{
	this->cameraPosition_ = position;
}

float Renderer::GetZoom()
{
	return zoom_;
}

void Renderer::SetZoom(float zoom)
{
	this->zoom_ = zoom;
}

glm::vec2 Renderer::GetResolution()
{
	return screenSize_;
}

glm::vec2 Renderer::EuToPixel(glm::vec2 size)
{
	return glm::vec2(
		(screenSize_.x / worldSize_.x) * size.x,
		(screenSize_.y / worldSize_.y) * size.y
	);
}

std::shared_ptr<Shader> Renderer::GetShader()
{
	return shader_;
}

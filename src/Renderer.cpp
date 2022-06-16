#include "Renderer.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/ext.hpp"

#include "Shader.hpp"
#include "FontAtlas.hpp"

glm::vec2 size = glm::vec2(40, 40);

// this allocates more memory than required since some characters (\n, \r, \t) produce no triangles
std::vector<VertexData> quadVertices(1000);
int totalQuads = 0;

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
	model = glm::translate(model, glm::vec3(80, 20, 0.0f));

	shader_->SetMatrix4("model", model);

	return window_;
}

void Renderer::BeginFrame()
{
	totalQuads = 0;

	glm::mat4 camera(1.0f);
	camera = glm::scale(camera, glm::vec3(zoom_, zoom_, 1.0f));
	camera = glm::translate(camera, glm::vec3(cameraPosition_, 0.f));
	shader_->SetMatrix4("camera", camera);

	int pixelRange = 2;
	glm::vec2  distanceField = glm::vec2(256, 256);
	glm::vec2 sizeInPixels = this->EuToPixel(size) * this->GetZoom();
	float screenPxRange = (sizeInPixels.x / distanceField.x) * pixelRange;
	shader_->SetFloat("screenPxRange", screenPxRange);
	shader_->SetVector2f("sizeInPixels", sizeInPixels);


}

void Renderer::EndFrame(FontAtlas& atlas)
{

	glBindBuffer(GL_ARRAY_BUFFER, atlas.GetVBO());

	auto byteSize = sizeof(VertexData) * 6 * totalQuads;
	glBufferData(GL_ARRAY_BUFFER, byteSize, quadVertices.data(), GL_DYNAMIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas.GetTexture());
	glBindVertexArray(atlas.GetQuadVAO());
	glDrawArrays(GL_TRIANGLES, 0, 6 * totalQuads);
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

void Renderer::DrawText(FontAtlas& atlas, std::string text, glm::vec3 position, float size, glm::vec4 color, bool center)
{
	// we render each letters as two triangles with 3 verts each
	const int vertsPerCharacter = 6;

	if (quadVertices.size() <= (totalQuads + text.length()) * vertsPerCharacter) // total letters
	{
		quadVertices.resize(quadVertices.size() * 2);
		printf("Resized to: %d\n", quadVertices.size());
	}

	shader_->SetVector4f("spriteColor", color);
	auto fontTexture = atlas.GetTexture();

	constexpr double tabWidthInEms = 2.0;

	double xoffset, yoffset;
	double fontLineHeight = 0.0, fontAscenderHeight = 0.0, fontDescenderHeight = 0.0;
	atlas.GetFontVerticalMetrics(fontTexture, fontLineHeight, fontAscenderHeight, fontDescenderHeight);

	yoffset = fontDescenderHeight - 1.0;

	size_t currentQuadIndex = 0;
	unsigned int currentLine = 0;
	char prevChar = 0;
	double cursorPos = 0.0;

	std::vector<double> lineWidths;
	lineWidths.emplace_back();
	size_t lineCount = 1;
	for (const char& c : text)
	{
		switch (c)
		{
		case '\r':
			lineWidths.back() = 0.0;
			break;
		case '\n': case '\f':
			lineWidths.emplace_back();
			lineCount++;
			break;
		case '\t':
		{
			unsigned int cursorPosRoundedDown = (unsigned int)lineWidths.back();
			lineWidths.back() = double(cursorPosRoundedDown) + tabWidthInEms - (fmod(cursorPosRoundedDown, tabWidthInEms));
			break;
		}
		default:
			lineWidths.back() += atlas.GetFontCharAdvance(fontTexture, c);
			break;
		}
	}



	for (const char& c : text)
	{
		xoffset = 0.0;

		if (c == '\n')
		{
			currentLine++;
			cursorPos = 0.0;
			continue;
		}
		if (c == '\r')
		{
			cursorPos = 0.0;
			continue;
		}
		if (c == '\t')
		{
			unsigned int cursorPosRoundedDown = (unsigned int)cursorPos;
			cursorPos = double(cursorPosRoundedDown) + tabWidthInEms - fmod(cursorPosRoundedDown, tabWidthInEms);
			continue;
		}

		if (center)
		{
			xoffset = -lineWidths[currentLine] / 2.0;
		}

		float l, r, b, t;
		atlas.GetFontCharUVBounds(fontTexture, c, l, r, b, t);
		quadVertices[totalQuads * vertsPerCharacter].in_uv = { l, t }; //lt
		quadVertices[totalQuads * vertsPerCharacter + 1].in_uv = { r, b };	//rb
		quadVertices[totalQuads * vertsPerCharacter + 2].in_uv = { l, b };	//lb
		quadVertices[totalQuads * vertsPerCharacter + 3].in_uv = { l, t };	//lt
		quadVertices[totalQuads * vertsPerCharacter + 4].in_uv = { r, t };	//rt
		quadVertices[totalQuads * vertsPerCharacter + 5].in_uv = { r, b };	//rb

		atlas.GetFontCharQuadBounds(fontTexture, c, l, r, b, t, prevChar);
		quadVertices[totalQuads * vertsPerCharacter].ep_position = position + glm::vec3(size * (l + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // lt
		quadVertices[totalQuads * vertsPerCharacter + 1].ep_position = position + glm::vec3(size * (r + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // rb
		quadVertices[totalQuads * vertsPerCharacter + 2].ep_position = position + glm::vec3(size * (l + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // lb
		quadVertices[totalQuads * vertsPerCharacter + 3].ep_position = position + glm::vec3(size * (l + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // lt
		quadVertices[totalQuads * vertsPerCharacter + 4].ep_position = position + glm::vec3(size * (r + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // rt
		quadVertices[totalQuads * vertsPerCharacter + 5].ep_position = position + glm::vec3(size * (r + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // rb

		quadVertices[totalQuads * vertsPerCharacter].color = color;
		quadVertices[totalQuads * vertsPerCharacter + 1].color = color;
		quadVertices[totalQuads * vertsPerCharacter + 2].color = color;
		quadVertices[totalQuads * vertsPerCharacter + 3].color = color;
		quadVertices[totalQuads * vertsPerCharacter + 4].color = color;
		quadVertices[totalQuads * vertsPerCharacter + 5].color = color;

		totalQuads++;
		prevChar = c;
		cursorPos += atlas.GetFontCharAdvance(fontTexture, c);
	}
}
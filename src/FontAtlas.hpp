#include <string>
#include <memory>
#include <map>

#include "glm/glm.hpp"

class Texture;

struct Glyph
{
	glm::vec2 start_;
	glm::vec2 end_;
	double advance_;

	Glyph()
	{

	}

	Glyph(glm::vec2 start, glm::vec2 end, double advance)
	{
		start_ = start;
		end_ = end;
		advance_ = advance;
	}
};

class Renderer;
class FontAtlas
{
	unsigned int quadVAO_;
	unsigned int vbo_;
	unsigned int texture_;

	std::map<std::pair<int, int>, double> kerning_;

	void Initialize(std::string fontFile);

	std::map<uint32_t, Glyph> glphys_;
	
public:
	FontAtlas(std::string fontFile);
	void DrawText(Renderer& renderer, std::string text);
};
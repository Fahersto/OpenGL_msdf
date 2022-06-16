#include <string>
#include <memory>
#include <map>
#include <vector>

#include "glm/glm.hpp"

class Texture;
typedef unsigned int TextureHandle;
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
struct VertexData
{
	glm::vec3 ep_position;
	glm::vec2 in_uv;
};

class Renderer;
class FontAtlas
{
	unsigned int quadVAO_;
	unsigned int vbo_;
	unsigned int texture_;


	static std::vector<VertexData> quadVertices;

	void Initialize(std::string fontFile);

	void GetFontCharUVBounds(TextureHandle atlas, uint32_t unicodeChar,
		float& out_l, float& out_r, float& out_b, float& out_t);

	void GetFontCharQuadBounds(TextureHandle atlas, uint32_t unicodeChar,
		float& out_l, float& out_r, float& out_b, float& out_t, uint32_t prevChar);

	double GetFontCharAdvance(TextureHandle atlas, uint32_t unicodeChar);

	void GetFontVerticalMetrics(TextureHandle atlas, double& out_lineHeight, double& out_ascenderHeight, double& out_descenderHeight);
	
public:
	FontAtlas(std::string fontFile);

	void DrawText(std::string text, glm::vec3 position, float size);
};
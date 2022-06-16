#include "Fontatlas.hpp"

#include "../msdf-atlas-gen/msdf-atlas-gen/msdf-atlas-gen.h"
#include "../msdf-atlas-gen/msdfgen/msdfgen.h"
#include "../msdf-atlas-gen/msdfgen/msdfgen-ext.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Renderer.hpp"
#include "Shader.hpp"





static std::map<TextureHandle, std::map<std::pair<uint32_t, uint32_t>, double>> fontKerns;
static std::map<TextureHandle, std::map<uint32_t, double>> fontAdvances;
static std::map<TextureHandle, std::map<uint32_t, std::tuple<float, float, float, float>>> fontUVBounds;
static std::map<TextureHandle, std::map<uint32_t, std::tuple<float, float, float, float>>> fontQuadBounds;
static std::map<TextureHandle, std::tuple<double, double, double>> fontVerticalMetrics; // line height, ascender height, descender height

// copied from: https://github.com/Chlumsky/msdf-atlas-gen
void FontAtlas::Initialize(std::string fontFilename) {
	using namespace msdf_atlas;
	// Initialize instance of FreeType library
	if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype()) {
		// Load font file
		if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontFilename.c_str())) {
			// Storage for glyph geometry and their coordinates in the atlas

			std::vector<msdf_atlas::GlyphGeometry> glyphs;
			// FontGeometry is a helper class that loads a set of glyphs from a single font.
			// It can also be used to get additional font metrics, kerning information, etc.
			msdf_atlas::FontGeometry fontGeometry = FontGeometry(&glyphs);
			// Load a set of character glyphs:
			// The second argument can be ignored unless you mix different font sizes in one atlas.
			// In the last argument, you can specify a charset other than ASCII.
			// To load specific glyph indices, use loadGlyphs instead.
			fontGeometry.loadCharset(font, 1.0, Charset::ASCII);
			// Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
			const double maxCornerAngle = 3.0;
			for (GlyphGeometry& glyph : glyphs)
				glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
			// TightAtlasPacker class computes the layout of the atlas.
			TightAtlasPacker packer;
			// Set atlas parameters:
			// setDimensions or setDimensionsConstraint to find the best value
			packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::POWER_OF_TWO_SQUARE);
			// setScale for a fixed size or setMinimumScale to use the largest that fits
			packer.setMinimumScale(32.0);
			// setPixelRange or setUnitRange
			packer.setPixelRange(2.0);
			packer.setMiterLimit(1.0);
			// Compute atlas layout - pack glyphs
			packer.pack(glyphs.data(), glyphs.size());
			// Get final atlas dimensions
			int width = 0, height = 0;
			packer.getDimensions(width, height);



			// The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
			ImmediateAtlasGenerator<
				float, // pixel type of buffer for individual glyphs depends on generator function
				3, // number of atlas color channels
				&msdfGenerator, // function to generate bitmaps for individual glyphs
				BitmapAtlasStorage<byte, 3> // class that stores the atlas bitmap
				// For example, a custom atlas storage class that stores it in VRAM can be used.
			> generator(width, height);
			// GeneratorAttributes can be modified to change the generator's default settings.
			GeneratorAttributes attributes;
			generator.setAttributes(attributes);
			generator.setThreadCount(4);
			// Generate atlas bitmap
			generator.generate(glyphs.data(), glyphs.size());










			msdfgen::BitmapConstRef<unsigned char, 3> bitmap = generator.atlasStorage();

			float vertices[] = {
				//vertex		  //uv in texture atlas
				0.0f, 1.0f, 0.0f, 0.0f,0.0f, // 1    
				1.0f, 0.0f, 0.0f, 0.0f,0.0f, //       
				0.0f, 0.0f, 0.0f, 0.0f,0.0f, // 3   2

				0.0f, 1.0f, 0.0f, 0.0f,0.0f, // 4   5
				1.0f, 1.0f, 0.0f, 0.0f,0.0f, //      
				1.0f, 0.0f, 0.0f, 0.0f,0.0f, //     6 
			};

			glGenVertexArrays(1, &quadVAO_);
			glGenBuffers(1, &vbo_);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glBindVertexArray(quadVAO_);

			// vertex 
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			// uv
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);


			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			// generate texture
			glGenTextures(1, &this->texture_);
			glBindTexture(GL_TEXTURE_2D, this->texture_);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);





			std::map<int, uint32_t> indexToCodePoint;
			for (const msdf_atlas::GlyphGeometry& glyph : glyphs)
			{
				indexToCodePoint[glyph.getIndex()] = glyph.getCodepoint();
				fontAdvances[this->texture_][glyph.getCodepoint()] = glyph.getAdvance();

				double l, b, r, t;
				glyph.getQuadAtlasBounds(l, b, r, t);
				l /= bitmap.width;
				r /= bitmap.width;
				b /= bitmap.height;
				t /= bitmap.height;
				fontUVBounds[this->texture_][glyph.getCodepoint()] = std::tuple(l, r, b, t);

				glyph.getQuadPlaneBounds(l, b, r, t);
				fontQuadBounds[this->texture_][glyph.getCodepoint()] = std::tuple(l, r, b, t);
			}

			msdfgen::FontMetrics metrics = fontGeometry.getMetrics();
			fontVerticalMetrics[this->texture_] = std::tuple
			(
				metrics.lineHeight,
				metrics.ascenderY,
				-metrics.descenderY
			);

			for (auto& [indicesKey, kernVal] : fontGeometry.getKerning())
			{
				std::pair<uint32_t, uint32_t> codePointsKey
				(
					indexToCodePoint[indicesKey.first],
					indexToCodePoint[indicesKey.second]
				);
				fontKerns[this->texture_][codePointsKey] = kernVal;
			}




			msdfgen::destroyFont(font);
		}
		msdfgen::deinitializeFreetype(ft);
	}
}

void FontAtlas::GetFontCharUVBounds(TextureHandle atlas, uint32_t unicodeChar,
	float& out_l, float& out_r, float& out_b, float& out_t)
{
	if (fontUVBounds.count(atlas) > 0)
	{
		if (fontUVBounds[atlas].count(unicodeChar) > 0)
		{
			out_l = std::get<0>(fontUVBounds[atlas][unicodeChar]);
			out_r = std::get<1>(fontUVBounds[atlas][unicodeChar]);
			out_b = std::get<2>(fontUVBounds[atlas][unicodeChar]);
			out_t = std::get<3>(fontUVBounds[atlas][unicodeChar]);
		}
		else
		{
			printf("error\n");
		}
	}
	else
	{
		printf("error\n");
	}
}

void FontAtlas::GetFontCharQuadBounds(TextureHandle atlas, uint32_t unicodeChar,
	float& out_l, float& out_r, float& out_b, float& out_t, uint32_t prevChar)
{
	if (fontQuadBounds.count(atlas) > 0)
	{
		if (fontQuadBounds[atlas].count(unicodeChar) > 0)
		{
			out_l = std::get<0>(fontQuadBounds[atlas][unicodeChar]);
			out_r = std::get<1>(fontQuadBounds[atlas][unicodeChar]);
			out_b = std::get<2>(fontQuadBounds[atlas][unicodeChar]);
			out_t = std::get<3>(fontQuadBounds[atlas][unicodeChar]);

			if (fontKerns[atlas].count(std::pair(unicodeChar, prevChar)) > 0)
			{
				out_l += fontKerns[atlas][std::pair(unicodeChar, prevChar)];
			}
		}
		else
		{
			printf("error\n");
		}
	}
	else
	{
		printf("error\n");
	}
}

double FontAtlas::GetFontCharAdvance(TextureHandle atlas, uint32_t unicodeChar)
{
	if (fontAdvances.count(atlas) > 0)
	{
		if (fontAdvances[atlas].count(unicodeChar) > 0)
		{
			return fontAdvances[atlas][unicodeChar];
		}
		else
		{
			printf("error\n");
			return 0.0;
		}
	}
	else
	{
		printf("error\n");
		return 0.0;
	}
}

void FontAtlas::GetFontVerticalMetrics(TextureHandle atlas, double& out_lineHeight, double& out_ascenderHeight, double& out_descenderHeight)
{
	if (fontVerticalMetrics.count(atlas) > 0)
	{
		out_lineHeight = std::get<0>(fontVerticalMetrics[atlas]);
		out_ascenderHeight = std::get<1>(fontVerticalMetrics[atlas]);
		out_descenderHeight = std::get<2>(fontVerticalMetrics[atlas]);
	}
	else
	{
		printf("error\n");
	}
}

FontAtlas::FontAtlas(std::string fontFile)
{
	this->Initialize(fontFile);
}


void FontAtlas::DrawText(Renderer& renderer, std::string text, glm::vec3 position, float size, glm::vec4 color, bool center)
{
	renderer.GetShader()->SetVector4f("spriteColor", color);

	constexpr double tabWidthInEms = 2.0;

	double xoffset, yoffset;
	double fontLineHeight = 0.0, fontAscenderHeight = 0.0, fontDescenderHeight = 0.0;
	GetFontVerticalMetrics(texture_, fontLineHeight, fontAscenderHeight, fontDescenderHeight);

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
			lineWidths.back() += GetFontCharAdvance(texture_, c);
			break;
		}
	}

	// this allocates more memory than required since some characters (\n, \r, \t) produce no triangles
	std::vector<VertexData> quadVertices(6 * text.length());

	const int vertsPerChar = 6; // we render them as two triangles with 3 verts each

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
		GetFontCharUVBounds(texture_, c, l, r, b, t);
		quadVertices[currentQuadIndex * vertsPerChar].in_uv		= { l, t }; //lt
		quadVertices[currentQuadIndex * vertsPerChar + 1].in_uv	= { r, b };	//rb
		quadVertices[currentQuadIndex * vertsPerChar + 2].in_uv	= { l, b };	//lb
		quadVertices[currentQuadIndex * vertsPerChar + 3].in_uv	= { l, t };	//lt
		quadVertices[currentQuadIndex * vertsPerChar + 4].in_uv	= { r, t };	//rt
		quadVertices[currentQuadIndex * vertsPerChar + 5].in_uv	= { r, b };	//rb

		GetFontCharQuadBounds(texture_, c, l, r, b, t, prevChar);
		quadVertices[currentQuadIndex * vertsPerChar].ep_position		= position + glm::vec3(size * (l + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // lt
		quadVertices[currentQuadIndex * vertsPerChar + 1].ep_position	= position + glm::vec3(size * (r + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // rb
		quadVertices[currentQuadIndex * vertsPerChar + 2].ep_position	= position + glm::vec3(size * (l + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // lb
		quadVertices[currentQuadIndex * vertsPerChar + 3].ep_position	= position + glm::vec3(size * (l + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // lt
		quadVertices[currentQuadIndex * vertsPerChar + 4].ep_position	= position + glm::vec3(size * (r + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // rt
		quadVertices[currentQuadIndex * vertsPerChar + 5].ep_position	= position + glm::vec3(size * (r + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // rb

		prevChar = c;
		currentQuadIndex++;
		cursorPos += GetFontCharAdvance(texture_, c);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	auto byteSize = sizeof(VertexData) * 6 * currentQuadIndex;
	glBufferData(GL_ARRAY_BUFFER, byteSize, quadVertices.data(), GL_DYNAMIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->texture_);
	glBindVertexArray(this->quadVAO_);
	glDrawArrays(GL_TRIANGLES, 0, 6 * currentQuadIndex);
}
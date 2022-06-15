#include "Fontatlas.hpp"

#include "../msdf-atlas-gen/msdf-atlas-gen/msdf-atlas-gen.h"
#include "../msdf-atlas-gen/msdfgen/msdfgen.h"
#include "../msdf-atlas-gen/msdfgen/msdfgen-ext.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Renderer.hpp"
#include "Shader.hpp"

double geometryScale_ = 0;
msdf_atlas::FontGeometry fontGeometry;
std::vector<msdf_atlas::GlyphGeometry> glyphs;

int bitmapWidth = 0;

msdf_atlas::GlyphGeometry GetGlyphByCodePoint(uint32_t codepoint)
{
    for (auto& g : glyphs)
    {
        if (g.getCodepoint() == codepoint)
        {
            return g;
        }
     }
}


// copied from: https://github.com/Chlumsky/msdf-atlas-gen
void FontAtlas::Initialize(std::string fontFilename) {
    using namespace msdf_atlas;
    // Initialize instance of FreeType library
    if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype()) {
        // Load font file
        if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontFilename.c_str())) {
            // Storage for glyph geometry and their coordinates in the atlas


            // FontGeometry is a helper class that loads a set of glyphs from a single font.
            // It can also be used to get additional font metrics, kerning information, etc.
            fontGeometry = FontGeometry(&glyphs);
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

            bitmapWidth = width;

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


            kerning_ = fontGeometry.getKerning();
            for (auto& glyph : glyphs)
            {
                Glyph g = Glyph(
                    glm::vec2(glyph.getBoxRect().x / (double)width, glyph.getBoxRect().y / (double)height),
                    glm::vec2((glyph.getBoxRect().x + glyph.getBoxRect().w) / (double)width, (glyph.getBoxRect().y + glyph.getBoxRect().h) / (double)height),
                    glyph.getAdvance()
                );
                glphys_.emplace(std::make_pair<>(glyph.getCodepoint(), g));
            }

            msdfgen::BitmapConstRef<unsigned char, 3> bitmap = generator.atlasStorage();

            float vertices[] = {
                // pos      
                0.0f, 1.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f,

                0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
            };

            glGenVertexArrays(1, &quadVAO_);
            glGenBuffers(1, &vbo_);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindVertexArray(quadVAO_);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            // generate
            glGenTextures(1, &this->texture_);
            glBindTexture(GL_TEXTURE_2D, this->texture_);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.pixels);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            msdfgen::destroyFont(font);
        }
        msdfgen::deinitializeFreetype(ft);
    }
}

FontAtlas::FontAtlas(std::string fontFile)
{
    this->Initialize(fontFile);
}

void FontAtlas::DrawText(Renderer& renderer, std::string text)
{
    std::shared_ptr<Shader> shader = renderer.GetShader();
    auto& lastGypth = GetGlyphByCodePoint('A');
    char lastChar = 'A';
    int index = 0;
    double cursor = 0;
    for (auto& character : text)
    {
        auto glyph = glphys_[character];

        auto start = glyph.start_;
        auto end = glyph.end_;
        auto delta = end - start;

        float vertices[] = {
            // first triangle  
            start.x, end.y, 0.0f,   // x
            end.x, start.y, 0.0f,   //     
            start.x, start.y, 0.0f, // x  x   
            //second triangle
            start.x, end.y, 0.0f,   // x  x
            end.x, end.y, 0.0f,     // 
            end.x, start.y, 0.0f,   //    x
        };

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

       
        shader->SetVector2f("renderingOffset", glm::vec2(
            -start.x - delta.x / 2 + cursor,
            -start.y - delta.y / 2
        ));

        auto& currentGlyph = GetGlyphByCodePoint(character);

      
        double advance;
        fontGeometry.getAdvance(advance, lastChar, character);
        auto kerning = fontGeometry.getKerning();

        double kerningVal = kerning[std::make_pair<>(lastGypth.getIndex(), currentGlyph.getIndex())];
        int boxWidth, boxHeight;
        currentGlyph.getBoxSize(boxWidth, boxHeight);

        double l, b, r, t;
        currentGlyph.getQuadAtlasBounds(l, b, r, t);

        int x, y, w, h;
        currentGlyph.getBoxRect(x, y, w, h);

        auto translate = currentGlyph.getBoxTranslate();
        
        currentGlyph.getQuadPlaneBounds(l, b, r, t);

        currentGlyph.getShape().bound(l,b,r,t);

        auto bounds = currentGlyph.getShape().getBounds();
        cursor += (advance * bitmapWidth) / renderer.GetResolution().x;

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glLineWidth(4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->texture_);
        glBindVertexArray(this->quadVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        lastGypth = currentGlyph;
        lastChar = character;
    }
}
#pragma once
// C++ standard libraries
#include <map>
#include <iostream>
#include <string>
#include <vector>

// Third_party libraries
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace quartz {

struct Character {
    glm::ivec2 size;
    glm::ivec2 bearing;
    
    glm::vec2 uvMin;
    glm::vec2 uvMax;

    unsigned int advance;
}; 

struct FontAtlas {
    GLuint textureID;
    unsigned int width  = 0;
    unsigned int height = 0;
    std::map<char, Character> characters;

    ~FontAtlas() {
        // if(textureID) {
        //     glDeleteTextures(1, &textureID);
        // }
    }
};

class FontManager {
    public:
        FontManager() {
            if (FT_Init_FreeType(&mFt)) {
                throw std::runtime_error("Could not init FreeType Library");
            }
        }
        ~FontManager() {
            FT_Done_FreeType(mFt);
        }

        const FontAtlas& GetFont(std::string& path, int size) {
            auto key = std::make_pair(path, size);
            auto it = mAtlasCache.find(key);

            if(it != mAtlasCache.end()) {
                return it->second;
            }

            FT_Face face;
            if(FT_New_Face(mFt, path.c_str(), 0, &face)) {
                throw std::runtime_error("Failed to load font: " + path);
            }

            FT_Set_Pixel_Sizes(face, 0, size);
            FontAtlas newAtlas;
            CreateAtlasTexture(face, newAtlas);
            FT_Done_Face(face);
            
            return mAtlasCache.emplace(key, std::move(newAtlas)).first->second;
        }

    private:
        void CreateAtlasTexture(FT_Face face, FontAtlas& atlas) {
            for (char c = 0; c < 126; c++) {
                if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
                FT_GlyphSlot glyph = face->glyph;
                atlas.width += glyph->bitmap.width + 1;
                atlas.height = std::max(atlas.height, static_cast<unsigned int>(glyph->bitmap.rows));
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glGenTextures(1, &atlas.textureID);
            glBindTexture(GL_TEXTURE_2D, atlas.textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas.width, atlas.height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            unsigned int xOffset = 0;
            
            for (char c = 0; c < 126; c++) {
                if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
                FT_GlyphSlot glyph = face->glyph;

                if(glyph->bitmap.buffer) {
                    // Flip letters so they stand the right way
                    std::vector<unsigned char> flippedBuffer(glyph->bitmap.rows * glyph->bitmap.width);
                    for (unsigned int row = 0; row < glyph->bitmap.rows; ++row) {
                        memcpy(
                            flippedBuffer.data() + row * glyph->bitmap.width,
                            glyph->bitmap.buffer + (glyph->bitmap.rows - 1 - row) * glyph->bitmap.width,
                            glyph->bitmap.width
                        );
                    }

                    glTexSubImage2D(
                        GL_TEXTURE_2D, 0,
                        xOffset, 0, 
                        glyph->bitmap.width, glyph->bitmap.rows, 
                        GL_RED, GL_UNSIGNED_BYTE, 
                        flippedBuffer.data()
                    );

                    Character ch = {
                        {glyph->bitmap.width, glyph->bitmap.rows},
                        {glyph->bitmap_left, glyph->bitmap_top},
                        {static_cast<float>(xOffset) / atlas.width, 0.0f},
                        {static_cast<float>(xOffset + glyph->bitmap.width) / atlas.width, static_cast<float>(glyph->bitmap.rows) / atlas.height},
                        static_cast<unsigned int>(glyph->advance.x)
                    };

                    atlas.characters[c] = ch;
                    xOffset += glyph->bitmap.width + 1;
                }
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }

    private:
        FT_Library mFt;
        std::map<std::pair<std::string, int>, FontAtlas> mAtlasCache; // FontPath FontSize

};

}
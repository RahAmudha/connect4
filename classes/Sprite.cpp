#include "Sprite.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <filesystem>

// Simple helper function to load an image into a OpenGL texture with common settings
bool Sprite::LoadTextureFromFile(const char* filename)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    std::filesystem::path resourcePath = std::filesystem::path("resources") / filename;
    std::string newFilename = resourcePath.string();
    unsigned char* image_data = stbi_load(newFilename.c_str(), &image_width, &image_height, NULL, 4);
    if (image_data == NULL) {
        _size = ImVec2(0, 0);
        std::cout << "Failed to load texture: " << newFilename << std::endl;
        return false;
    }
    _texture = _loadTextureFromMemory(image_data, image_width, image_height);
    stbi_image_free(image_data);
    if (_texture == 0) {
        _size = ImVec2(0, 0);
        return false;
    }
    _size = ImVec2((float)image_width, (float)image_height);
    return true;
}

void Sprite::setHighlighted(bool highlighted)
{
	if (highlighted != _highlighted) {
		_highlighted = highlighted;
	}
}

bool Sprite::highlighted()
{
	return _highlighted;
}

#include "../imgui/imgui_impl_opengl3_loader.h"

ImTextureID Sprite::_loadTextureFromMemory(const unsigned char *image_data, int image_width, int image_height)
{
    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

    return static_cast<ImTextureID>(image_texture);
}

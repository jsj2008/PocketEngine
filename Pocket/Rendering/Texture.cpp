#include "Texture.hpp"
#include "OpenGL.hpp"
#include "ImageLoader.hpp"

using namespace Pocket;

Texture::Texture() {
	width = 0;
	height = 0;
	texture = 0;
}

Texture::~Texture() {
	Free();
}

int Texture::GetWidth() {
	return width;
}

int Texture::GetHeight() {
	return height;
}

GLuint Texture::GetHandle() {
	return texture;
}

void Texture::LoadFromMemory(unsigned char *data, int size) {
    ImageLoader::TryLoadImageFromData(data, size, [this] (unsigned char* pixels, int width, int height) {
        CreateFromBuffer(pixels, width, height, GL_RGBA);
    });
}

void Texture::LoadFromFile(const std::string& filename) {
    ImageLoader::TryLoadImage(filename, [this] (unsigned char* pixels, int width, int height) {
        CreateFromBuffer(pixels, width, height, GL_RGBA);
    });
}

void Texture::CreateFromBuffer(unsigned char *buffer, int width, int height, GLenum pixelFormat) {
    
    Free();
    
    this->width = width;
    this->height = height;
    
    ASSERT_GL(glGenTextures(1, &texture));
	ASSERT_GL(glBindTexture(GL_TEXTURE_2D, texture));
    
    ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	
	//ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	//ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    
    ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    
    //ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    //ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_POINTS));
    //ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    //ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    
    ASSERT_GL(glTexImage2D(GL_TEXTURE_2D, 0, pixelFormat, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, buffer));
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
    GenerateMipmaps();
}

void Texture::DisableMipmapping() {
    ASSERT_GL(glBindTexture(GL_TEXTURE_2D, texture));
    ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    ASSERT_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
}

void Texture::GenerateMipmaps() {
    ASSERT_GL(glGenerateMipmap(GL_TEXTURE_2D));
}

void Texture::Free() {
	if (texture) {
		ASSERT_GL(glDeleteTextures(0, &texture));
		texture = 0;
	}
}

void Texture::SaveToPng(const std::string &path, GLenum pixelFormat) {
#ifdef EMSCRIPTEN
#elif IPHONE
#else
    ASSERT_GL(glBindTexture(GL_TEXTURE_2D, texture));
    unsigned char* pixels = new unsigned char[width * height * 4];
    ASSERT_GL(glGetTexImage(GL_TEXTURE_2D, 0, pixelFormat, GL_UNSIGNED_BYTE, pixels));
#endif
    /*std::vector<unsigned char> out;
    LodePNG::Encoder encoder;
	encoder.encode(out, (const unsigned char*)pixels, width, height);
    delete[] pixels;
	LodePNG::saveFile(out, path);
    */
}

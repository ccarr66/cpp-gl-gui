#pragma once
#include "GLOBALS.h"

#if (ENABLE_GUI)
#include "Renderer.h"

class Texture
{
private:
	unsigned int m_RendererID;
	string m_Filepath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
public:
	Texture(const string& path);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline const int& GetWidth() const { return m_Width; }
	inline const int& GetHeight() const { return m_Height; }

};
#endif //(ENABLE_GUI)


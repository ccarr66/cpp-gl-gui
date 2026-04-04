#pragma once
#include "GLOBALS.h"

#if (ENABLE_GUI)
#include <cstdint>

class VertexBuffer
{
private:
	unsigned int m_RendererID;
public:
	VertexBuffer() = delete;
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
};
#endif //(ENABLE_GUI)

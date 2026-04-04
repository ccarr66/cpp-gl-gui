#include "VertexBuffer.h"
#if (ENABLE_GUI)

#include "Renderer.h"

#include <GL/glew.h>

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{ 
	//generate buffer to store rendering data used in draw-call
	//auto buffer = unsigned int{ 0 };
	GLCall(glGenBuffers(1, &m_RendererID));
	//select generated buffer & desginates it as just a memory block
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	//put data in buffer
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
	//select generated buffer & desginates it as just a memory block
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const
{
	//select generated buffer & desginates it as just a memory block
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
#endif //(ENABLE_GUI)


#include "GLOBALS.h"
#if (ENABLE_GUI)
#include "IndexBuffer.h"
#include "Renderer.h"

#include <GL/glew.h>

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
	: m_Count(count)
{
	
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));

	//generate buffer to store rendering data used in draw-call
	//auto buffer = unsigned int{ 0 };
	GLCall(glGenBuffers(1, &m_RendererID));
	//select generated buffer & desginates it as just a memory block
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
	//put data in buffer
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
	
}

IndexBuffer::~IndexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void IndexBuffer::Bind() const
{
	//select generated buffer & desginates it as just a memory block
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
}

void IndexBuffer::Unbind() const
{
#if (USE_GLFW)
	//select generated buffer & desginates it as just a memory block
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
#else
#endif
}

#endif // ENABLE_GUI


#include "VertexArray.h"

#if (ENABLE_GUI)

#include "Renderer.h"
#include "VertexBufferLayout.h"

#include <GL/glew.h>


VertexArray::VertexArray()
{
	//creates Vertex Array
	GLCall(glGenVertexArrays(1, &m_RendererID));
	GLCall(glBindVertexArray(m_RendererID));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	Bind();
	vb.Bind();

	const auto& elements = layout.GetElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		//enables index 0 of the current vertex array to point to current vertex buffer
		GLCall(glEnableVertexAttribArray(i));
		//binds current vertex array element to current vertex buffer
		GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)(uintptr_t)(offset)));

		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray(0));
}

#endif //(ENABLE_GUI)

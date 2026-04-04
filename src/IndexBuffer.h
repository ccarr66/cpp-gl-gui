#pragma once
#include "GLOBALS.h"
#include <cstdint>

class IndexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Count;
public:
	IndexBuffer() = delete;
	IndexBuffer(const unsigned int* data, unsigned int count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	inline const unsigned int& GetCount() const { return m_Count; }
};


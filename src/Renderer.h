#pragma once
#include "GLOBALS.h"
#if (ENABLE_GUI)


#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

class Renderer 
{
private:
public:
	void Clear() const;
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);

};

#endif

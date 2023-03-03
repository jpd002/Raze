#include "gs_buffers.h"
#include <cstdio>
#include <malloc.h>
#include <exception>
#include <cstring>

GsBuffer::~GsBuffer()
{
	free(m_data);
}

void GsBuffer::SetData(size_t size, const void *data, BufferUsageType type)
{
	printf("GsBuffer::SetData(size = %d);\n", static_cast<int>(size));
	if(m_size != size)
	{
		free(m_data);
		m_data = memalign(0x10, size);
		if(!m_data)
		{
			printf("Failed to alloc buffer.\n");
			throw std::bad_alloc();
		}
	}
	if(data)
	{
		memcpy(m_data, data, size);
	}
	m_size = size;
}

void GsBuffer::SetSubData(size_t offset, size_t size, const void *data)
{
}

void* GsBuffer::Lock(unsigned int size)
{
	return m_data;
}

void GsBuffer::Unlock()
{
}

void GsBuffer::Map()
{
	map = m_data;
}

void GsBuffer::Unmap()
{
	map = nullptr;
}

void GsBuffer::Resize(size_t newsize)
{
	SetData(newsize, nullptr, BufferUsageType::Static);
}

void GsVertexBuffer::SetFormat(int numBindingPoints, int numAttributes, size_t stride, const FVertexBufferAttribute *attrs)
{
	m_stride = stride;
	for(int i = 0; i < numAttributes; i++)
	{
		const auto& attr = attrs[i];
		assert(attr.binding == 0);
		switch(attr.location)
		{
		case VATTR_VERTEX:
			assert(attr.format == VFmt_Float3);
			m_positionOffset = attr.offset;
			break;
		}
	}
}

void GsDataBuffer::BindRange(FRenderState* state, size_t start, size_t length)
{
}

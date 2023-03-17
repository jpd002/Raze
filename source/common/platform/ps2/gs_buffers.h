#pragma once

#include "buffers.h"

class GsBuffer : virtual public IBuffer
{
public:
	GsBuffer() = default;
	virtual ~GsBuffer();
	void SetData(size_t size, const void *data, BufferUsageType type) override;
	void SetSubData(size_t offset, size_t size, const void *data) override;
	void* Lock(unsigned int size) override;
	void Unlock() override;
	void Map() override;
	void Unmap() override;
	void Resize(size_t newsize) override;

protected:
	void* m_data = nullptr;
	size_t m_size = 0;
};

class GsVertexBuffer : public IVertexBuffer, public GsBuffer
{
public:
	GsVertexBuffer() = default;
	virtual ~GsVertexBuffer() = default;
	void SetFormat(int numBindingPoints, int numAttributes, size_t stride, const FVertexBufferAttribute *attrs) override;

	size_t m_positionOffset = -1;
	size_t m_texCoordOffset = -1;
	size_t m_stride = -1;
};

class GsIndexBuffer : public IIndexBuffer, public GsBuffer
{
public:
	GsIndexBuffer() = default;
	virtual ~GsIndexBuffer() = default;
};

class GsDataBuffer : public IDataBuffer, public GsBuffer
{
public:
	GsDataBuffer(int bindingPoint);
	virtual ~GsDataBuffer() = default;
	void BindRange(FRenderState* state, size_t start, size_t length) override;

private:
	int m_bindingPoint = 0;
};

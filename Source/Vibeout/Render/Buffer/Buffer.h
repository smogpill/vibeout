// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Renderer;

class Buffer
{
public:
	struct Setup
	{
		VkDeviceSize _size;
		VkBufferUsageFlags _usage;
		VkMemoryPropertyFlags _memProps = 0;
	};
	Buffer(const Renderer& renderer, const Setup& setup, bool& result);
	~Buffer();

	void* Map();
	template <class T>
	T* Map();
	void Unmap();
	VkDeviceAddress GetDeviceAddress() const;
	VkBuffer GetBuffer() const { return _buffer; }
	VkDeviceMemory GetMemory() const { return _memory; }
	void Write(const void* buffer, uint32 size);
	size_t GetSize() const { return _size; }

private:
	bool Init(const Setup& setup);

	const Renderer& _renderer;
	VkBuffer _buffer = nullptr;
	VkDeviceMemory _memory = nullptr;
	VkDeviceAddress _address = 0;
	size_t _size = 0;
	bool _mapped = false;
};

template <class T>
T* Buffer::Map()
{
	VO_ASSERT(sizeof(T) <= _size);
	return (T*)Map();
}

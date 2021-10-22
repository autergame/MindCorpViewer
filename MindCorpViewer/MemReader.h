//author https://github.com/autergame
#ifndef _MEMREADER_H_
#define _MEMREADER_H_

#include <vector>

void my_assert(const char* exp, const char* func, const char* file, unsigned int line)
{
#ifdef _DEBUG
	__debugbreak();
#endif
	fprintf(stderr, "ERROR: %s %s\n%s %d\n", func, exp, file, line);
	scanf_s("Press enter to exit.");
	_Exit(EXIT_FAILURE);
}
#define myassert(expression) if (expression) { my_assert(#expression, __FUNCTION__, __FILE__, __LINE__); }

class CharMemVector
{
public:
	size_t m_Pointer = 0;
	std::vector<uint8_t> m_Array;

	CharMemVector() {}
	~CharMemVector() {}

	template<typename T>
	void MemWrite(T value)
	{
		MemWrite((void*)&value, sizeof(T));
	}

	void MemWrite(void* value, size_t size)
	{
		m_Array.insert(m_Array.end(), (uint8_t*)value, (uint8_t*)value + size);
	}


	template<typename T>
	T MemRead()
	{
		return *(T*)MemRead(sizeof(T));
	}

	void* MemRead(size_t bytes)
	{
		uint8_t* buffer = new uint8_t[bytes];
		MemRead(buffer, bytes);
		return buffer;
	}

	void MemRead(void* buffer, size_t bytes)
	{
		auto arrayIt = m_Array.begin() + m_Pointer;
		std::_Adl_verify_range(arrayIt, arrayIt + bytes);

		myassert(memcpy(buffer, m_Array.data() + m_Pointer, bytes) != buffer)
		m_Pointer += bytes;
	}
};

#endif //_MEMREADER_H_
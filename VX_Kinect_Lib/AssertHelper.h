#pragma once

#include <cassert>

#define ASSERT _ASSERTE

#ifdef _DEBUG
#define VERIFY ASSERT

struct Tracer
{
	char const *myFileName;
	unsigned int myLine;

	Tracer(char const *filename, unsigned int const line)
		: myFileName(filename), myLine(line) {}

	template<typename ... Args>
	auto operator()(wchar_t const *format, Args ... args) const -> void
	{
		wchar_t buffer[256];
		auto count = swprintf_s(buffer, L"%S(%d): ", myFileName, myLine);
		ASSERT(-1 != count);

		va_list args;
		va_start(args, format);

		ASSERT(-1 != _snwprintf_s(buffer + count, _countof(buffer) - count, _countof(buffer) - count - 1, format, args...));

		va_end(args);
		OutputDebugStringW(buffer);
	}
};

#define TRACE Tracer(__FILE__, __LINE__)
#else
#define VERIFY(expression) (expression)
#define TRACE __noop
#endif
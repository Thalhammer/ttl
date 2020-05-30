#include <gtest/gtest.h>

#include "ttl/dynlib.h"

using ttl::dynlib;

#ifdef _WIN32
TEST(DynLibTest, WindowsLoadLib) {
	dynlib lib;
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_TRUE(lib.open("kernel32.dll"));
	ASSERT_TRUE(lib.errormsg().empty());

	auto ptr = lib.get_function<DWORD WINAPI()>("GetCurrentProcessId");
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_NE(nullptr, ptr);
	auto res = ptr();

	ASSERT_EQ(GetCurrentProcessId(), res);
}

TEST(DynLibTest, WindowsLoadLibFailedLib) {
	dynlib lib;
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_FALSE(lib.open("nonexistent.dll"));
	ASSERT_FALSE(lib.errormsg().empty());

	auto ptr = lib.get_function<DWORD WINAPI()>("GetCurrentProcessId");
	ASSERT_EQ(nullptr, ptr);
}

TEST(DynLibTest, WindowsLoadLibFailedFunction) {
	dynlib lib;
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_TRUE(lib.open("kernel32.dll"));
	ASSERT_TRUE(lib.errormsg().empty());

	auto ptr = lib.get_function<DWORD WINAPI()>("NonExistentFunction");
	ASSERT_FALSE(lib.errormsg().empty());
	ASSERT_EQ(nullptr, ptr);
}

TEST(DynLibTest, WindowsLoadLibSymbolTable) {
	dynlib lib;
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_TRUE(lib.open("kernel32.dll"));
	ASSERT_TRUE(lib.errormsg().empty());

	std::set<std::string> symbols;
	ASSERT_TRUE(lib.get_symbols(symbols));

	// Check if function exists
	ASSERT_TRUE(symbols.count("GetCurrentProcessId"));
}

#else
TEST(DynLibTest, LinuxLoadLib) {
	dynlib lib;
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_TRUE(lib.open("libm.so.6"));
	ASSERT_TRUE(lib.errormsg().empty());

	auto ptr = lib.get_function<double(double)>("sin");
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_NE(nullptr, ptr);
	auto res = ptr(0);

	ASSERT_FLOAT_EQ(0, res);
}

TEST(DynLibTest, LinuxLoadLibFailedLib) {
	dynlib lib;
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_FALSE(lib.open("nonexistent.so"));
	ASSERT_FALSE(lib.errormsg().empty());

	auto ptr = lib.get_function<double(double)>("sin");
	ASSERT_EQ(nullptr, ptr);
}

TEST(DynLibTest, LinuxLoadLibFailedFunction) {
	dynlib lib;
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_TRUE(lib.open("libm.so.6"));
	ASSERT_TRUE(lib.errormsg().empty());

	auto ptr = lib.get_function<void()>("NonExistentFunction");
	ASSERT_FALSE(lib.errormsg().empty());
	ASSERT_EQ(nullptr, ptr);
}

TEST(DynLibTest, LinuxLoadLibSymbolTable) {
	dynlib lib;
	ASSERT_TRUE(lib.errormsg().empty());
	ASSERT_TRUE(lib.open("libm.so.6"));
	ASSERT_TRUE(lib.errormsg().empty());

	std::set<std::string> symbols;
	ASSERT_TRUE(lib.get_symbols(symbols));
	ASSERT_NE(0, symbols.size());
	for (auto& e : symbols) {
		auto ptr = lib.get_function(e);
		ASSERT_TRUE(ptr != nullptr);
	}
	// Check if function exists
	ASSERT_TRUE(symbols.count("sin"));
}
#endif

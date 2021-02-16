cmake_minimum_required (VERSION 3.8)

#project ("test_00_basic_usage")

add_executable (test_00_basic_usage 
	"../../Source/thread_local_variable_manager.h"
	"../../Source/thread_local_variable_manager.inl"
	"pch.h"
	"pch.cpp"
	"test_main.cpp"
)

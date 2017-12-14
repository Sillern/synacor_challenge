cmake_minimum_required(VERSION 3.3)
project (synacor_machine)
set(EXECUTABLE_OUTPUT_PATH "bin")

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unknown-pragmas -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wno-long-long -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-conversion -Wno-error=sign-conversion -Wno-error=unused-private-field")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wno-long-long -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-conversion -Wno-error=sign-conversion -Wno-error=unused-private-field")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DDEBUG")

add_executable(synacor_machine src/virtual_machine.c)

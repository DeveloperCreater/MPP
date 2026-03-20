cmake_minimum_required(VERSION 3.14)
project(mpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(mpp
    main.cpp
    lexer.cpp
    parser.cpp
    value.cpp
    interpreter.cpp
)

if(MSVC)
    target_compile_options(mpp PRIVATE /W4)
else()
    target_compile_options(mpp PRIVATE -Wall -Wextra)
endif()

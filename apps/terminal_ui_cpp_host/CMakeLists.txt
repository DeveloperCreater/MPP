cmake_minimum_required(VERSION 3.20)
project(mpp_terminal_ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# This is a Windows-only sample using WebView2.
if (NOT WIN32)
  message(FATAL_ERROR "This app is Windows-only (WebView2).")
endif()

add_executable(TerminalUI WIN32
  main.cpp

  # M++ C++ runtime sources (reuse existing interpreter)
  ../../cpp/lexer.cpp
  ../../cpp/parser.cpp
  ../../cpp/value.cpp
  ../../cpp/interpreter.cpp
)

target_include_directories(TerminalUI PRIVATE
  .
  ../../cpp
)

# WebView2: we use the SDK headers (must be available).
# Recommended: install Microsoft.Web.WebView2 via Visual Studio NuGet, or add the SDK include path here.
#
# If you installed the WebView2 SDK manually, set WEBVIEW2_SDK_DIR to the folder containing "include/WebView2.h".
if (DEFINED WEBVIEW2_SDK_DIR)
  target_include_directories(TerminalUI PRIVATE "${WEBVIEW2_SDK_DIR}/include")
endif()

if (MSVC)
  target_compile_options(TerminalUI PRIVATE /W4)
else()
  target_compile_options(TerminalUI PRIVATE -Wall -Wextra)
endif()

# Copy web/ alongside the built exe so it can load local files.
add_custom_command(TARGET TerminalUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/web"
          "$<TARGET_FILE_DIR:TerminalUI>/web"
)


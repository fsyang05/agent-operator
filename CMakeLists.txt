cmake_minimum_required(VERSION 3.23)
project(AgentOperator)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FTXUI
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v6.1.9  # Replace with a version, tag, or commit hash
)
FetchContent_MakeAvailable(ftxui)

add_executable(run
    src/main.cpp
    src/agent.cpp
    src/tmux_session.cpp
    src/ui/agent_panel.cpp
)

target_include_directories(run PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_compile_options(run PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_definitions(run PRIVATE ENABLE_DEBUG)

target_link_libraries(run
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

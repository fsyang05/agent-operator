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

add_executable(agent-operator
    src/main.cpp
    src/app/app.cpp
    src/ui/tui.cpp
    src/model/tmux_session.cpp
)

target_include_directories(agent-operator PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_compile_options(agent-operator PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_definitions(agent-operator PRIVATE ENABLE_DEBUG)

target_link_libraries(agent-operator
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

install(TARGETS agent-operator DESTINATION bin)

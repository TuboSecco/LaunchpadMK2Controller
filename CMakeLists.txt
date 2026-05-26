cmake_minimum_required(VERSION 3.16)
project(LaunchpadController CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Collect sources ───────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/ConfigLoader.cpp
    src/MidiManager.cpp
    src/LedRenderer.cpp
    src/PageManager.cpp
    src/LightshowEngine.cpp
)

# ── Windows Icon Configuration ────────────────────────────────────────────────
# Includiamo il file di risorse solo se compiliamo su Windows
if(WIN32)
    list(APPEND SOURCES resources/resources.rc)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

# ── Windows subsystem: console (keeps stdout visible) ────────────────────────
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()

# ── Include path ──────────────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE src)

# ── Link winmm (MIDI) and shell32 (ShellExecute) ─────────────────────────────
# Both are already listed via #pragma comment(lib,...) in headers, but
# explicitly listing them here makes MinGW-w64 builds work too.
target_link_libraries(${PROJECT_NAME} PRIVATE winmm shell32)

# ── Compiler warnings ─────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Optional: static runtime (truly standalone .exe on MSVC) ─────────────────
# Uncomment to link the CRT statically (no VC++ Redistributable needed):
# set_property(TARGET ${PROJECT_NAME} PROPERTY
#     MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

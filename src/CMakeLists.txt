# Files defined here are added to the library regardless of the target system.
set(
        SOURCES
        "sertop/worker.h"
        "sertop/worker.cpp"
        "utils/buffers.h"
        "utils/buffers.cpp"
        "utils/config.h"
        "utils/config.cpp"
        "utils/exceptions.h"
        "utils/exceptions.cpp"
        "waterproof/message.h"
        "waterproof/message.cpp"
        "waterproof/server.h"
        "waterproof/server.cpp"
        "conductor.h"
        "conductor.cpp"
)

# Files defined here are only added to the library when building for a Windows system.
set(
        SOURCES_WIN32
        "win/api.h"
        "win/api_wrapper.h"
        "win/api_wrapper.cpp"
        "sertop/worker_win.cpp"
        "waterproof/server_win.cpp"
)

# Files defined here are only added to the library when building for a (near) POSIX-compliant system (i.e. macOS or
# Linux).
set(
        SOURCES_POSIX
        "posix/api.h"
        "posix/api_wrapper.h"
        "posix/api_wrapper.cpp"
        "sertop/worker_posix.cpp"
        "waterproof/server_posix.cpp"
)

if (UNIX)
    add_compile_definitions(WPWRAPPER_POSIX=1)
elseif (MINGW OR MSVC)
    # Explicitly target Windows 10. This allows us to use features that are only available on newer versions of Windows.
    add_definitions(-D_WIN32_WINNT=0x0A00 -DWIN32_LEAN_AND_MEAN)
    add_compile_definitions(WPWRAPPER_WIN=1)
else()
    message(FATAL_ERROR "Unknown platform cannot continue")
endif()

# Library target.
add_library(
        wpwrapper_lib
        STATIC
        ${SOURCES}
        $<$<PLATFORM_ID:Windows>:${SOURCES_WIN32}>
        $<$<PLATFORM_ID:Linux>:${SOURCES_POSIX}>
        $<$<PLATFORM_ID:Darwin>:${SOURCES_POSIX}>
)

# Dependencies are linked publicly so they can be used by the executable too.
# On Unix systems, we need to link pthread too.
target_link_libraries(wpwrapper_lib PUBLIC spdlog::spdlog fmt::fmt nlohmann_json::nlohmann_json)
if (UNIX)
    target_link_libraries(wpwrapper_lib PUBLIC pthread)
elseif (MINGW)
    target_link_libraries(wpwrapper_lib PUBLIC pthread wsock32 ws2_32)
elseif (MSVC)
    target_link_libraries(wpwrapper_lib PUBLIC wsock32 ws2_32)
endif ()

# Executable target.
add_executable(wpwrapper "main.cpp")
if (UNIX)
    # Don't link statically on UNIX platforms
    target_link_libraries(wpwrapper PUBLIC wpwrapper_lib -pthread)
else ()
    target_link_libraries(wpwrapper PUBLIC wpwrapper_lib -static -static-libgcc -static-libstdc++ -pthread)
endif ()

# Copy the binary to ../bin after each build.
add_custom_command(
        TARGET wpwrapper
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wpwrapper>
        ${PROJECT_SOURCE_DIR}/bin
)

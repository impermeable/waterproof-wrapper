# Fetch, build and configure external dependencies.

include(FetchContent)
include(helper.cmake)

message(STATUS "Checking project dependencies")

# fmt
fetchcontent_declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 8.1.1
)

fetchcontent_getproperties(fmt)
fetchcontenthelper_check(fmt)

# spdlog
fetchcontent_declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.9.2
)

set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)

fetchcontent_getproperties(spdlog)
fetchcontenthelper_check(spdlog)

# json
fetchcontent_declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.6.1
)

fetchcontent_getproperties(json)
fetchcontenthelper_check(json)

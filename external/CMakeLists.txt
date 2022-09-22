# Fetch, build and configure external dependencies.

include(FetchContent)
include(helper.cmake)

message(STATUS "Checking project dependencies")

# GoogleTest and GoogleMock
fetchcontent_declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.8.1
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

fetchcontent_getproperties(googletest)
fetchcontenthelper_check(googletest)

# fmt
fetchcontent_declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 5.3.0
)

fetchcontent_getproperties(fmt)
fetchcontenthelper_check(fmt)

# spdlog
fetchcontent_declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x
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

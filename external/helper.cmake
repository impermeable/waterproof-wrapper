include(FetchContent)

# Check if a declared external project has been fetched 'populated'. If not,
# the project is fetched and added.
function(fetchcontenthelper_check name)
    message(STATUS "Searching for ${name}...")
    if(NOT ${name}_POPULATED)
        message(STATUS "Searching for ${name} -- missing!")

        message(STATUS "Fetching ${name}...")
        fetchcontent_populate(${name})
        message(STATUS "Fetching ${name} -- done!")

        message(STATUS "Installing ${name}...")
        add_subdirectory(
                ${${name}_SOURCE_DIR}
                ${${name}_BINARY_DIR}
                EXCLUDE_FROM_ALL
        )
        message(STATUS "Installing ${name} -- done!")
    else()
        message(STATUS "Searching for ${name} -- found!")
    endif()
endfunction()
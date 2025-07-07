
cmake_minimum_required(VERSION 3.10)

project(sync-daemon C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    main.c
)

# Create executable
add_executable(sync-daemon ${SOURCES})

# For POSIX functionality
target_link_libraries(sync-daemon PRIVATE m)

# Required for syslog, etc.
target_link_libraries(sync-daemon PRIVATE rt)
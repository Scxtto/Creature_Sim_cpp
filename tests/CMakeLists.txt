include(FetchContent)

# Fixes CMake policy warning for URL-based FetchContent downloads
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(CreatureSimTests
  test_smoke.cpp
  test_simrandom.cpp
)

target_include_directories(CreatureSimTests PRIVATE
  ${PROJECT_SOURCE_DIR}
)

target_link_libraries(CreatureSimTests PRIVATE
  CreatureSimLib
  GTest::gtest_main
)


include(GoogleTest)

# Automatically registers each TEST() with CTest
gtest_discover_tests(CreatureSimTests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
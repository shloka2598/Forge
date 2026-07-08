file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    *.cpp
)

add_executable(forge_tests ${TEST_SOURCES})

target_include_directories(forge_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(forge_tests
    PRIVATE
        forge_core
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)

catch_discover_tests(forge_tests)
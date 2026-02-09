add_executable(unit_tests
    test_main.cpp
    test_chunkprocessor.cpp
)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/headers)

target_link_libraries(unit_tests PRIVATE
    DownloadCore
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    Qt6::Test
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

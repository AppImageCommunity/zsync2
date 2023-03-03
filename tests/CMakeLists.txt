enable_testing()
include(CTest)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)

add_executable(test_zsutil test_zsutil.cpp)
target_link_libraries(test_zsutil PRIVATE libzsync2 GTest::gtest cpr)
gtest_discover_tests(test_zsutil)

add_executable(test_zshash test_zshash.cpp)
target_link_libraries(test_zshash PRIVATE libzsync2 GTest::gtest cpr)
gtest_discover_tests(test_zshash)

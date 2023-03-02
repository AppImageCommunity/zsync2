enable_testing()
include(CTest)

set(CMAKE_CXX_STANDARD 11)

add_executable(test_zsutil test_zsutil.cpp)
target_link_libraries(test_zsutil PRIVATE libzsync2 GTest::gtest cpr)
add_test(NAME test_zsutil COMMAND test_zsutil)

add_executable(test_zshash test_zshash.cpp)
target_link_libraries(test_zshash PRIVATE libzsync2 GTest::gtest cpr)
add_test(NAME test_zshash COMMAND test_zshash)

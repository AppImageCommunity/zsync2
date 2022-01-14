set(CMAKE_MINIMUM_REQUIRED_VERSION 3.2)

enable_testing()
include(CTest)

set(CMAKE_CXX_STANDARD 11)

include_directories(../src)

include_directories(${CPR_INCLUDE_DIRS})

add_executable(test_zsutil test_zsutil.cpp)
target_link_libraries(test_zsutil PRIVATE libzsync2 gtest cpr)

add_test(NAME test_zsutil COMMAND test_zsutil)

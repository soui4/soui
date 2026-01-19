# Copyright (c) 2021 Soui
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set(TEST_NAME network_test)

set(SOURCE_FILES
    test_network.cpp
)

add_executable(${TEST_NAME} ${SOURCE_FILES})

target_link_libraries(${TEST_NAME} 
    gtest
    ${CMAKE_THREAD_LIBS_INIT}
)

if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${TEST_NAME} swinx)
endif()

target_include_directories(${TEST_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/third-part/gtest/include
    ${CMAKE_SOURCE_DIR}/SOUI/include
)

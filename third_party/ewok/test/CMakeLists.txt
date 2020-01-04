# ------------------------------------------------------------------------------
# ${TEST_NAME} executable
set(TEST_NAME unittests_${PROJECT_NAME})
add_executable(${TEST_NAME}
  "ring_buffer_base_test.cpp"
  "ed_ring_buffer_test.cpp"
  "raycast_ring_buffer_test.cpp"
  "unittest_main.cpp")
target_include_directories(${TEST_NAME} PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(${TEST_NAME}
  catch
  ewok)

# ------------------------------------------------------------------------------
# Unit tests
enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

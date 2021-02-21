
set(TARGET_NAME "shadow_memory_unittest")

add_executable(${TARGET_NAME}
        ${GTEST_MAIN_FILE}
        shadow_memory_unittest.cpp
        ../shadow_memory.c
        shadow_memory_medium_mock.cpp
        )

set_target_properties(${TARGET_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_include_directories(${TARGET_NAME} PRIVATE
        ../
        )

target_link_libraries(${TARGET_NAME}
        ${GMOCK_LIB}
        ${GTEST_LIB}
        pthread
        )

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME} ${GTEST_ARGS})

add_dependencies(${UNITTEST_TARGET_NAME} ${TARGET_NAME})

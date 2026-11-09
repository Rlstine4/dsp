# One binary per test file; add new tests here
set(DSP_TEST_SOURCES
        test_sample.cpp
        test_circular_buffer.cpp
        test_fir.cpp
)

# Catch2 v2 helper for test discovery (provides catch_discover_tests)
if(Catch2_SOURCE_DIR)
    list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/contrib")
elseif(Catch2_DIR)
    list(APPEND CMAKE_MODULE_PATH "${Catch2_DIR}")
endif()
include(Catch)

foreach(test_source ${DSP_TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)

    add_executable(${test_name} ${test_source} main.cpp)
    target_link_libraries(${test_name} PRIVATE ${LIB_TARGET}::${LIB_TARGET} Catch2::Catch2)

    catch_discover_tests(${test_name})
endforeach()

add_executable(dsp_tests test_sample.cpp)

target_link_libraries(dsp_tests PRIVATE DSP Catch2::Catch2)

# Catch2 v2 helper for test discovery (provides catch_discover_tests)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
include(Catch)

catch_discover_tests(dsp_tests)

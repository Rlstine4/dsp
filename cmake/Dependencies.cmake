include(CPM)

CPMAddPackage(
        NAME nlohmann_json
        VERSION 3.9.1
        GITHUB_REPOSITORY nlohmann/json
        OPTIONS
        "JSON_BuildTests OFF"
)

CPMAddPackage(
        NAME matplotlib-cpp
        VERSION 1.0.0
        GIT_REPOSITORY https://github.com/lava/matplotlib-cpp.git
        GIT_TAG master
)

CPMAddPackage(
        NAME Catch2
        VERSION 2.13.10
        GITHUB_REPOSITORY catchorg/Catch2
        OPTIONS
        "CATCH_BUILD_TESTING OFF"
        "CATCH_BUILD_EXAMPLES OFF"
        "CATCH_BUILD_EXTRA_TESTS OFF"
        "CATCH_ENABLE_WERROR OFF"
)
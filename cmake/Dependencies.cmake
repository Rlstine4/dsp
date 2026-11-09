include(CPM)

set(CPM_USE_LOCAL_PACKAGES ${USE_LOCAL_PACKAGES})

CPMAddPackage(
    NAME expected
    GITHUB_REPOSITORY TartanLlama/expected
    GIT_TAG v1.3.1
    OPTIONS
        "EXPECTED_BUILD_TESTS OFF"
        "EXPECTED_BUILD_PACKAGE OFF"
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

CPMAddPackage(
    NAME pocketfft
    GIT_REPOSITORY https://github.com/mreineck/pocketfft.git
    GIT_TAG c90e55b3d529f8efa40ed01a20de22405f45fc65
    DOWNLOAD_ONLY YES
)

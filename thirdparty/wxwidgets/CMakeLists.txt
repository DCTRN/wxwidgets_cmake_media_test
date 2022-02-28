# check if wxWidgets is already installed in the system - using CMake's built in script FindwxWidgets
# find_package(wxWidgets COMPONENTS base gl core media gst REQUIRED)


# if (wxWidgets_FOUND)
#     message(STATUS "Found preinstalled wxWidgets libraries at ${wxWidgets_LIBRARIES}")
#     add_library(wxWidgets_external INTERFACE)
# else()
    message(STATUS "Preinstalled wxWidgets not found.")
    message(STATUS "Will download and install wxWidgets in ${STAGED_INSTALL_PREFIX}")
    
    include(ExternalProject)
    ExternalProject_Add(wxWidgets_external
        GIT_REPOSITORY
        https://github.com/wxWidgets/wxWidgets.git
        GIT_TAG
        31adb7d55b1f68d8b4c1a6ddd80d731aaf71f3b8
        UPDATE_COMMAND
        ""
        CONFIGURE_COMMAND
        /home/dctr/projects/wx_cmake_template/build/subprojects/Source/wxWidgets_external/configure --with-opengl --with-gtk=3 --enable-mediactrl
        BUILD_COMMAND
        make -j14
        INSTALL_COMMAND
        sudo make install
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${STAGED_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
        -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
        -DwxBUILD_SHARED=OFF
        CMAKE_CACHE_ARGS
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        TEST_AFTER_INSTALL
        0
        DOWNLOAD_NO_PROGRESS
        1
        LOG_CONFIGURE
        1
        LOG_BUILD
        1
        LOG_INSTALL
        1
        )

    set(wxWidgets_ROOT_DIR ${STAGED_INSTALL_PREFIX})
    file(TO_NATIVE_PATH "${wxWidgets_ROOT_DIR}" wxWidgets_ROOT_DIR)
    set(wxWidgets_ROOT_DIR ${wxWidgets_ROOT_DIR} CACHE INTERNAL "wxWidgets installation dir")

    set (ENV_WX_CONFIG ${STAGED_INSTALL_PREFIX}/bin/wx-config)
    file (TO_NATIVE_PATH "${ENV_WX_CONFIG}" ENV_WX_CONFIG)
    set(ENV_WX_CONFIG ${ENV_WX_CONFIG} CACHE INTERNAL "wx-config dir")

    # target_link_libraries(wxWidgets_external ${GSTREAMER_LIBRARIES})

# endif()
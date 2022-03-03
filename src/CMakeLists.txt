cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(wx_cmake_template_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# hack for buggy CMake's FindwxWidgets script
if (DEFINED ENV_WX_CONFIG)
    set (ENV{WX_CONFIG} ${ENV_WX_CONFIG})
endif()

set(wxWidgets_USE_STATIC 1)


find_package(PkgConfig) #finding pkg-config is a helper tool
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-video-1.0 gstreamer-player-1.0)
include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
)
link_directories(
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
)

# find_package(wxWidgets REQUIRED)
find_package(wxWidgets COMPONENTS base gl core media REQUIRED)

message(xWidgets_LIBRARIES: ${wxWidgets_LIBRARIES})
message(GSTREAMER_LIBRARIES: ${GSTREAMER_LIBRARIES})

set(SRCS 
    main.cpp
    wxMediaPlayerApp.cpp
    wxMediaPlayerFrame.cpp
    wxMediaPlayerListCtrl.cpp
    wxMediaPlayerNotebookPage.cpp
    wxMediaPlayerTimer.cpp
    wxPlayListDropTarget.cpp
)

include(${wxWidgets_USE_FILE})

if (APPLE)
    # create bundle on apple compiles
    add_executable(main MACOSX_BUNDLE ${SRCS} )
    # Set a custom plist file for the app bundle - needed for Mac OS Retina display
    set_target_properties(main PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
else()
    # the WIN32 is needed for Windows in order for it to look for WinMain
    # instead of the main function. This is ignored on other systems,
    # so it works on all platforms
    add_executable(main WIN32 ${SRCS})
endif()

target_link_libraries(main PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(main PRIVATE ${GSTREAMER_LIBRARIES})
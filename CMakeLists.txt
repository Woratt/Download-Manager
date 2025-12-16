cmake_minimum_required(VERSION 3.19)
project(DownloadManager LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Sql)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(DownloadManager
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    downloadmanager.h downloadmanager.cpp
    downloaditem.h downloaditem.cpp
    threadpool.h threadpool.cpp
    downloadtask.h downloadtask.cpp
    toogle.h toogle.cpp
    downloaddatabase.h downloaddatabase.cpp
    downloadrecord.h downloadrecord.cpp
    downloaditemadapter.h downloaditemadapter.cpp
    downloadtypes.h
)

target_link_libraries(DownloadManager
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        Qt::Sql
)
target_link_libraries(DownloadManager PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)
target_link_libraries(DownloadManager PRIVATE Qt6::Core)
target_link_libraries(DownloadManager PRIVATE Qt6::Core)
target_link_libraries(DownloadManager PRIVATE Qt6::Core)

include(GNUInstallDirs)

install(TARGETS DownloadManager
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET DownloadManager
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

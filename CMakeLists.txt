cmake_minimum_required(VERSION 3.19)
project(DownloadManager LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Sql)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(DownloadManager
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    headers/mainwindow.h
    headers/downloadmanager.h src/downloadmanager.cpp
    headers/downloaditem.h src/downloaditem.cpp
    headers/threadpool.h src/threadpool.cpp
    headers/downloadtask.h src/downloadtask.cpp
    headers/toogle.h src/toogle.cpp
    headers/downloaddatabase.h src/downloaddatabase.cpp
    headers/downloadrecord.h src/downloadrecord.cpp
    headers/downloaditemadapter.h src/downloaditemadapter.cpp
    headers/downloadtypes.h
    headers/networkmanager.h src/networkmanager.cpp
    headers/chunkprocessor.h src/chunkprocessor.cpp
    headers/storagemanager.h src/storagemanager.cpp
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

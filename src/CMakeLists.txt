
set(CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/headers/downloadmanager.h
    ${CMAKE_SOURCE_DIR}/headers/threadpool.h
    ${CMAKE_SOURCE_DIR}/headers/downloadtask.h
    ${CMAKE_SOURCE_DIR}/headers/downloaddatabase.h
    ${CMAKE_SOURCE_DIR}/headers/downloadrecord.h
    ${CMAKE_SOURCE_DIR}/headers/downloaditemadapter.h
    ${CMAKE_SOURCE_DIR}/headers/networkmanager.h
    ${CMAKE_SOURCE_DIR}/headers/chunkprocessor.h
    ${CMAKE_SOURCE_DIR}/headers/storagemanager.h
    ${CMAKE_SOURCE_DIR}/headers/mainwindow.h
    ${CMAKE_SOURCE_DIR}/headers/downloaditem.h
    ${CMAKE_SOURCE_DIR}/headers/toogle.h
)

set(CORE_SOURCES
    downloadmanager.cpp
    threadpool.cpp
    downloadtask.cpp
    downloaddatabase.cpp
    downloadrecord.cpp
    downloaditemadapter.cpp
    networkmanager.cpp
    chunkprocessor.cpp
    storagemanager.cpp
    main.cpp
    mainwindow.cpp
    downloaditem.cpp
    toogle.cpp
)

add_library(DownloadCore STATIC ${CORE_HEADERS} ${CORE_SOURCES})

target_link_libraries(DownloadCore PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    Qt6::Widgets
)

set(APP_HEADERS
    ${CMAKE_SOURCE_DIR}/headers/mainwindow.h
    ${CMAKE_SOURCE_DIR}/headers/downloaditem.h
    ${CMAKE_SOURCE_DIR}/headers/toogle.h
)

set(APP_SOURCES
    main.cpp
)

qt_add_executable(DownloadManagerApp WIN32 MACOSX_BUNDLE ${APP_SOURCES})

target_link_libraries(DownloadManagerApp PRIVATE
    DownloadCore
    Qt6::Widgets
    Qt6::Network
)

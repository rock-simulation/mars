project(main_gui)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "The library provides a small core for a modular gui implementation.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()

# Widgets finds its own dependencies (QtGui and QtCore).
find_package(Qt5Widgets REQUIRED)

# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES})
set(BKA "blud")
message(error " da ${BKA} ${QT_LIBRARIES} ${Qt5Widgets_DEFINITIONS}")
# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

pkg_check_modules(LIB_MANAGER "lib_manager")
include_directories(${LIB_MANAGER_INCLUDE_DIRS})
link_directories(${LIB_MANAGER_LIBRARY_DIRS})

# we only need the header
pkg_check_modules(CFG_MANAGER "cfg_manager")
include_directories(${CFG_MANAGER_INCLUDE_DIRS})

include_directories(
    src
    src/interfaces
    src/qtpropertybrowser
    src/qtpropertybrowser/extension
)


set(SOURCES 
    src/BaseWidget.cpp
    src/MainGUI.cpp
    src/MyQDockWidget.cpp
    src/MyQMainWindow.cpp
    src/MyQMdiArea.cpp
    src/PropertyDialog.cpp

    src/qtpropertybrowser/qtbuttonpropertybrowser.cpp
    src/qtpropertybrowser/qteditorfactory.cpp
    src/qtpropertybrowser/qtgroupboxpropertybrowser.cpp
    src/qtpropertybrowser/qtpropertybrowser.cpp
    src/qtpropertybrowser/qtpropertybrowserutils.cpp
    src/qtpropertybrowser/qtpropertymanager.cpp
    src/qtpropertybrowser/qttreepropertybrowser.cpp
    src/qtpropertybrowser/qtvariantproperty.cpp

    src/qtpropertybrowser/extension/fileedit.cpp
    src/qtpropertybrowser/extension/fileeditfactory.cpp
    src/qtpropertybrowser/extension/filepathmanager.cpp
    src/qtpropertybrowser/extension/variantfactory.cpp
    src/qtpropertybrowser/extension/variantmanager.cpp
)

set(HEADERS
    src/BaseWidget.h
    src/MainGUI.h
    src/MyQDockWidget.h
    src/MyQMainWindow.h
    src/MyQMdiArea.h
    src/PropertyCallback.h
    src/PropertyDialog.h

    src/interfaces/GuiInterface.h
    src/interfaces/MenuInterface.h

    src/qtpropertybrowser/qtbuttonpropertybrowser.h
    src/qtpropertybrowser/qteditorfactory.h
    src/qtpropertybrowser/qtgroupboxpropertybrowser.h
    src/qtpropertybrowser/qtpropertybrowser.h
    src/qtpropertybrowser/qtpropertybrowserutils.h
    src/qtpropertybrowser/qtpropertymanager.h
    src/qtpropertybrowser/qttreepropertybrowser.h
    src/qtpropertybrowser/qtvariantproperty.h

    src/qtpropertybrowser/extension/fileedit.h
    src/qtpropertybrowser/extension/fileeditfactory.h
    src/qtpropertybrowser/extension/filepathmanager.h
    src/qtpropertybrowser/extension/variantfactory.h
    src/qtpropertybrowser/extension/variantmanager.h
)

set(QT_MOC_HEADER
    src/BaseWidget.h
    src/MainGUI.h
    src/MyQDockWidget.h
    src/MyQMainWindow.h
    src/MyQMdiArea.h
    src/PropertyDialog.h

    src/qtpropertybrowser/qtpropertybrowserutils.h

    src/qtpropertybrowser/extension/fileedit.h
    src/qtpropertybrowser/extension/fileeditfactory.h
    src/qtpropertybrowser/extension/filepathmanager.h
    src/qtpropertybrowser/extension/variantfactory.h
    src/qtpropertybrowser/extension/variantmanager.h
)

set(QT_OWN_MOC
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtbuttonpropertybrowser.moc
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qteditorfactory.moc
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtgroupboxpropertybrowser.moc
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtpropertybrowser.moc
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtpropertymanager.moc
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qttreepropertybrowser.moc
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtvariantproperty.moc
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qteditorfactory.moc1
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qttreepropertybrowser.moc1
    ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtpropertymanager.moc1
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qtbuttonpropertybrowser.h
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtbuttonpropertybrowser.moc
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qteditorfactory.h
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qteditorfactory.moc
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qtgroupboxpropertybrowser.h
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtgroupboxpropertybrowser.moc
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qtpropertybrowser.h
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtpropertybrowser.moc
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qtpropertymanager.h
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtpropertymanager.moc
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qttreepropertybrowser.h
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qttreepropertybrowser.moc
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qtvariantproperty.h
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtvariantproperty.moc
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qteditorfactory.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qteditorfactory.moc1
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qttreepropertybrowser.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qttreepropertybrowser.moc1
)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser/qtpropertymanager.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser/qtpropertymanager.moc1
)

qt5_wrap_cpp( QT_MOC_HEADER_SRC ${QT_MOC_HEADER} )

include_directories(${CMAKE_CURRENT_BINARY_DIR}/src/qtpropertybrowser)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${QT_MOC_HEADER_SRC} ${QT_OWN_MOC})

qt5_use_modules(${PROJECT_NAME} Widgets)

target_link_libraries(${PROJECT_NAME}
                      ${LIB_MANAGER_LIBRARIES}
                      ${QT5Widgets_LIBRARIES}
 )


if(WIN32)
  set(LIB_INSTALL_DIR bin) # .dll are in PATH, like executables
else(WIN32)
  set(LIB_INSTALL_DIR lib)
endif(WIN32)


set(_INSTALL_DESTINATIONS
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION lib
)


# Install the library into the lib folder
install(TARGETS ${PROJECT_NAME} ${_INSTALL_DESTINATIONS})

# Install headers into mars include directory
install(FILES ${HEADERS} DESTINATION include/mars/${PROJECT_NAME})

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)



# CMake file for the QT GUI apps
# set output dir
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(XDG_APPS_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/applications")
set(XDG_APPS_PIXMAPS_DIR "${CMAKE_INSTALL_DATAROOTDIR}/icons/")

include_directories(
	${PROJECT_SOURCE_DIR}/src
	appcmn_qt
	widgets_qt
)

#
# Find Qt
#
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

message(STATUS "Qt: Using Qt Version ${QT_VERSION_MAJOR}")

# required components
find_package(Qt${QT_VERSION_MAJOR} NO_MODULE REQUIRED COMPONENTS Gui Widgets Concurrent Xml SerialPort)

# optional components
find_package(
    Qt${QT_VERSION_MAJOR} NO_MODULE QUIET
    OPTIONAL_COMPONENTS
    WebEngineWidgets
    WebKitWidgets
    LinguistTools
)
if(TARGET Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
   message(STATUS "Qt: Found optional QtWebEngine")
   add_definitions(-DQWEBENGINE)
else()
   if(TARGET Qt${QT_VERSION_MAJOR}::WebKitWidgets)
      message(STATUS "Qt: Found optional QtWebKit")
      add_definitions(-DQWEBKIT)
   endif()
endif()

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC TRUE)
set(CMAKE_AUTOUIC TRUE)

# translation
if((${QT_VERSION_MAJOR} EQUAL 6) AND (${Qt6LinguistTools_FOUND}))
   # add translation support, zh_CN for Chinese
   set(LANGUAGES en de zh_CN)
   message(STATUS "Qt: Adding translation: ${LANGUAGES}")
   if(QT_VERSION GREATER_EQUAL 6.7)
       qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ${LANGUAGES})
   else()
       # Qt 6.5 has no I18N_TRANSLATED_LANGUAGES; translations are handled
       # explicitly by qt_add_lrelease/qt_add_resources in each app.
       qt_standard_project_setup()
   endif()
endif()

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    if(MSVC)
        add_compile_options(/Zc:__cplusplus)
    endif()
    if(MINGW)
        # prevent 'file too big' and 'too many sections' errors with Qt6
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
    endif()
endif()

# try to set up the menu system
if (LINUX)
    find_program(XDG-MIME_EXECUTABLE xdg-mime)
    find_program(XDG-DESKTOP-MENU_EXECUTABLE xdg-desktop-menu)
endif()

if(WIN32)
	add_definitions(-DWIN32)
endif()

#add_subdirectory(appcmn_qt)
#set_property(DIRECTORY appcmn_qt PROPERTY QT_EXCLUDE_FROM_TRANSLATION ON)
add_subdirectory(rtkconv_qt)
add_subdirectory(rtkget_qt)
add_subdirectory(rtklaunch_qt)
add_subdirectory(rtknavi_qt)
add_subdirectory(rtkplot_qt)
add_subdirectory(rtkpost_qt)
add_subdirectory(srctblbrows_qt)
add_subdirectory(strsvr_qt)

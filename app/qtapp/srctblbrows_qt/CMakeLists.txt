add_executable(srctblbrows_qt
    browsmain.cpp
    srctblbrows.cpp

    browsmain.ui

    ../appcmn_qt/staoptdlg.cpp
    ../appcmn_qt/mapview.cpp
    ../appcmn_qt/mapviewopt.cpp
    ../appcmn_qt/aboutdlg.cpp
    ../appcmn_qt/helper.cpp
    ../appcmn_qt/mntpoptdlg.cpp
    ../appcmn_qt/viewer.cpp
    ../appcmn_qt/vieweropt.cpp

    ../appcmn_qt/staoptdlg.ui
    ../appcmn_qt/mapview.ui
    ../appcmn_qt/mapviewopt.ui
    ../appcmn_qt/aboutdlg.ui
    ../appcmn_qt/mntpoptdlg.ui
    ../appcmn_qt/viewer.ui
    ../appcmn_qt/vieweropt.ui

    ../appcmn_qt/appcmn_qt.qrc
    ../icon/resources.qrc


    srctblbrows_qt.rc)
if(QT_VERSION GREATER_EQUAL 6.5)
        qt_add_lrelease(srctblbrows_qt TS_FILES rtklib_zh_CN.ts
                        QM_FILES_OUTPUT_VARIABLE srctblbrows_qt_qm)
        # the .qm lives in the build dir, so force a flat resource alias
        set_source_files_properties(${srctblbrows_qt_qm} PROPERTIES
                        QT_RESOURCE_ALIAS "rtklib_zh_CN.qm")
        qt_add_resources(srctblbrows_qt "i18n" PREFIX "/i18n" FILES ${srctblbrows_qt_qm})
endif()
target_link_libraries(srctblbrows_qt PRIVATE
        rtklib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Concurrent
)

if(TARGET Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
   target_link_libraries(srctblbrows_qt PRIVATE Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
else()
  if((${Qt5WebKitWidgets_FOUND}) OR (${Qt6WebKitWidgets_FOUND}))
     target_link_libraries(srctblbrows_qt PRIVATE Qt${QT_VERSION_MAJOR}::WebKitWidgets)
  endif()
endif()

set_target_properties(srctblbrows_qt PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
install(TARGETS srctblbrows_qt RUNTIME DESTINATION bin BUNDLE DESTINATION bin)
if (UNIX)
   install(FILES srctblbrows_qt.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
   install(FILES ../icon/srctblbrows.png DESTINATION ${XDG_APPS_PIXMAPS_DIR})
endif()

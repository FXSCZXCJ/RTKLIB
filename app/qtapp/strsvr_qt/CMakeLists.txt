add_executable(strsvr_qt
    convdlg.cpp
    strsvr.cpp
    svrmain.cpp
    svroptdlg.cpp
    mondlg.cpp

    convdlg.ui
    svrmain.ui
    svroptdlg.ui
    mondlg.ui

    ../appcmn_qt/aboutdlg.cpp
    ../appcmn_qt/tcpoptdlg.cpp
    ../appcmn_qt/serioptdlg.cpp
    ../appcmn_qt/cmdoptdlg.cpp
    ../appcmn_qt/console.cpp
    ../appcmn_qt/fileoptdlg.cpp
    ../appcmn_qt/refdlg.cpp
    ../appcmn_qt/mntpoptdlg.cpp
    ../appcmn_qt/keydlg.cpp
    ../appcmn_qt/helper.cpp

    ../appcmn_qt/tcpoptdlg.ui
    ../appcmn_qt/serioptdlg.ui
    ../appcmn_qt/aboutdlg.ui
    ../appcmn_qt/cmdoptdlg.ui
    ../appcmn_qt/console.ui
    ../appcmn_qt/fileoptdlg.ui
    ../appcmn_qt/refdlg.ui
    ../appcmn_qt/mntpoptdlg.ui
    ../appcmn_qt/keydlg.ui

    ../appcmn_qt/appcmn_qt.qrc
    ../icon/resources.qrc


    strsvr_qt.rc)
if(QT_VERSION GREATER_EQUAL 6.5)
        qt_add_lrelease(strsvr_qt TS_FILES rtklib_zh_CN.ts
                        QM_FILES_OUTPUT_VARIABLE strsvr_qt_qm)
        # the .qm lives in the build dir, so force a flat resource alias
        set_source_files_properties(${strsvr_qt_qm} PROPERTIES
                        QT_RESOURCE_ALIAS "rtklib_zh_CN.qm")
        qt_add_resources(strsvr_qt "i18n" PREFIX "/i18n" FILES ${strsvr_qt_qm})
endif()
target_link_libraries(strsvr_qt PRIVATE
        rtklib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::SerialPort
)

set_target_properties(strsvr_qt PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
install(TARGETS strsvr_qt RUNTIME DESTINATION bin BUNDLE DESTINATION bin)
if (UNIX)
   install(FILES strsvr_qt.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
   install(FILES ../icon/strsvr.png DESTINATION ${XDG_APPS_PIXMAPS_DIR})
endif()

add_executable(rtkconv_qt
    codeopt.cpp 
    convmain.cpp 
    convopt.cpp 
    rtkconv.cpp 
    startdlg.cpp

    codeopt.ui 
    convopt.ui 
    startdlg.ui
    convmain.ui

    ../appcmn_qt/aboutdlg.cpp 
    ../appcmn_qt/keydlg.cpp 
    ../appcmn_qt/timedlg.cpp 
    ../appcmn_qt/viewer.cpp 
    ../appcmn_qt/vieweropt.cpp 
    ../appcmn_qt/glofcndlg.cpp 
    ../appcmn_qt/mntpoptdlg.cpp 
    ../appcmn_qt/helper.cpp 
    ../appcmn_qt/freqdlg.cpp

    ../appcmn_qt/aboutdlg.ui
    ../appcmn_qt/keydlg.ui
    ../appcmn_qt/timedlg.ui
    ../appcmn_qt/viewer.ui
    ../appcmn_qt/vieweropt.ui
    ../appcmn_qt/glofcndlg.ui
    ../appcmn_qt/mntpoptdlg.ui
    ../appcmn_qt/freqdlg.ui

    ../appcmn_qt/appcmn_qt.qrc 
    ../icon/resources.qrc


    rtkconv_qt.rc)
if(QT_VERSION GREATER_EQUAL 6.5)
	qt_add_lrelease(rtkconv_qt TS_FILES rtklib_zh_CN.ts
                        QM_FILES_OUTPUT_VARIABLE rtkconv_qt_qm)
        # the .qm lives in the build dir, so force a flat resource alias
        set_source_files_properties(${rtkconv_qt_qm} PROPERTIES
                        QT_RESOURCE_ALIAS "rtklib_zh_CN.qm")
        qt_add_resources(rtkconv_qt "i18n" PREFIX "/i18n" FILES ${rtkconv_qt_qm})
endif()

target_link_libraries(rtkconv_qt PRIVATE 
	rtklib
	Qt${QT_VERSION_MAJOR}::Core 
	Qt${QT_VERSION_MAJOR}::Gui 
	Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(rtkconv_qt PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
install(TARGETS rtkconv_qt RUNTIME DESTINATION bin BUNDLE DESTINATION bin)

if (UNIX)
   install(FILES rtkconv_qt.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
   install(FILES ../icon/rtkconv.png DESTINATION ${XDG_APPS_PIXMAPS_DIR})
endif()

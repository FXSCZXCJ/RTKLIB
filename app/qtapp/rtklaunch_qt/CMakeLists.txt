add_executable(rtklaunch_qt
    launchmain.cpp
    main.cpp
    launchoptdlg.cpp

    launchmain.ui
    launchoptdlg.ui

    ../appcmn_qt/appcmn_qt.qrc
    ../icon/resources.qrc


    rtklaunch_qt.rc)
if(QT_VERSION GREATER_EQUAL 6.5)
        qt_add_lrelease(rtklaunch_qt TS_FILES rtklib_zh_CN.ts
                        QM_FILES_OUTPUT_VARIABLE rtklaunch_qt_qm)
        # the .qm lives in the build dir, so force a flat resource alias
        set_source_files_properties(${rtklaunch_qt_qm} PROPERTIES
                        QT_RESOURCE_ALIAS "rtklib_zh_CN.qm")
        qt_add_resources(rtklaunch_qt "i18n" PREFIX "/i18n" FILES ${rtklaunch_qt_qm})
endif()
target_link_libraries(rtklaunch_qt PRIVATE
        rtklib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(rtklaunch_qt PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
install(TARGETS rtklaunch_qt RUNTIME DESTINATION bin BUNDLE DESTINATION bin)
if (UNIX)
   install(FILES rtklaunch_qt.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
   install(FILES ../icon/rtklaunch.png DESTINATION ${XDG_APPS_PIXMAPS_DIR})
endif()

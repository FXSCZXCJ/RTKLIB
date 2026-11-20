add_executable(rtkpost_qt
    kmzconv.cpp
    postmain.cpp
    rtkpost.cpp

    kmzconv.ui
    postmain.ui

    ../appcmn_qt/navi_post_opt.cpp
    ../appcmn_qt/aboutdlg.cpp
    ../appcmn_qt/freqdlg.cpp
    ../appcmn_qt/keydlg.cpp
    ../appcmn_qt/maskoptdlg.cpp
    ../appcmn_qt/refdlg.cpp
    ../appcmn_qt/viewer.cpp
    ../appcmn_qt/vieweropt.cpp
    ../appcmn_qt/helper.cpp
    ../appcmn_qt/timedlg.cpp

    ../widgets_qt/doubleunitvalidator.cpp
    ../widgets_qt/scientificspinbox.cpp
    ../widgets_qt/excludedsatellitevalidator.cpp

    ../appcmn_qt/navi_post_opt.ui
    ../appcmn_qt/keydlg.ui
    ../appcmn_qt/freqdlg.ui
    ../appcmn_qt/maskoptdlg.ui
    ../appcmn_qt/refdlg.ui
    ../appcmn_qt/viewer.ui
    ../appcmn_qt/vieweropt.ui
    ../appcmn_qt/aboutdlg.ui
    ../appcmn_qt/timedlg.ui

    ../appcmn_qt/appcmn_qt.qrc
    ../icon/resources.qrc


    rtkpost_qt.rc)
if(QT_VERSION GREATER_EQUAL 6.5)
        qt_add_lrelease(rtkpost_qt TS_FILES rtklib_zh_CN.ts
                        QM_FILES_OUTPUT_VARIABLE rtkpost_qt_qm)
        # the .qm lives in the build dir, so force a flat resource alias
        set_source_files_properties(${rtkpost_qt_qm} PROPERTIES
                        QT_RESOURCE_ALIAS "rtklib_zh_CN.qm")
        qt_add_resources(rtkpost_qt "i18n" PREFIX "/i18n" FILES ${rtkpost_qt_qm})
endif()
target_link_libraries(rtkpost_qt PRIVATE
        rtklib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(rtkpost_qt PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
install(TARGETS rtkpost_qt RUNTIME DESTINATION bin BUNDLE DESTINATION bin)
if (UNIX)
   install(FILES rtkpost_qt.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
   install(FILES ../icon/rtkpost.png DESTINATION ${XDG_APPS_PIXMAPS_DIR})
endif()

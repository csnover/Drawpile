# Qt tries to provide compatibility between versions 5 and 6 on its own, but
# that stuff isn't available when building against Qt 5.12, so we can't use it.
# So we have to define alias targets and macros ourselves instead.

if("${QT_VERSION}" EQUAL 5)
    add_library(DP::QtCore ALIAS Qt5::Core)
    add_library(DP::QtNetwork ALIAS Qt5::Network)
    add_library(DP::QtSql ALIAS Qt5::Sql)
    if(TARGET Qt5::AndroidExtras)
        add_library(DP::QtAndroidExtras ALIAS Qt5::AndroidExtras)
    endif()
    if(TARGET Qt5::Gui)
        add_library(DP::QtGui ALIAS Qt5::Gui)
    endif()
    if(TARGET qt5keychain)
        add_library(DP::QtKeychain ALIAS qt5keychain)
    endif()
    if(TARGET Qt5::Multimedia)
        add_library(DP::QtMultimedia ALIAS Qt5::Multimedia)
    endif()
    if(TARGET Qt5::Svg)
        add_library(DP::QtSvg ALIAS Qt5::Svg)
    endif()
    if(TARGET Qt5::Test)
        add_library(DP::QtTest ALIAS Qt5::Test)
    endif()
    if(TARGET Qt5::Widgets)
        add_library(DP::QtWidgets ALIAS Qt5::Widgets)
    endif()

    macro(dp_qt_add_resources)
        qt5_add_resources(${ARGN})
    endmacro()

    macro(dp_qt_wrap_ui)
        qt5_wrap_ui(${ARGN})
    endmacro()

	if(Qt5LinguistTools_FOUND)
        set(DP_QtLinguistTools_FOUND ON)
        macro(dp_qt_add_translation)
            qt5_add_translation(${ARGN})
        endmacro()
    else()
        unset(DP_QtLinguistTools_FOUND)
    endif()

elseif("${QT_VERSION}" EQUAL 6)
    add_library(DP::QtCore ALIAS Qt6::Core)
    add_library(DP::QtNetwork ALIAS Qt6::Network)
    add_library(DP::QtSql ALIAS Qt6::Sql)
    if(TARGET Qt6::AndroidExtras)
        add_library(DP::QtAndroidExtras ALIAS Qt6::AndroidExtras)
    endif()
    if(TARGET Qt6::Gui)
        add_library(DP::QtGui ALIAS Qt6::Gui)
    endif()
    if(TARGET qt6keychain)
        add_library(DP::QtKeychain ALIAS qt6keychain)
    endif()
    if(TARGET Qt6::Multimedia)
        add_library(DP::QtMultimedia ALIAS Qt6::Multimedia)
    endif()
    if(TARGET Qt6::Svg)
        add_library(DP::QtSvg ALIAS Qt6::Svg)
    endif()
    if(TARGET Qt6::Test)
        add_library(DP::QtTest ALIAS Qt6::Test)
    endif()
    if(TARGET Qt6::Widgets)
        add_library(DP::QtWidgets ALIAS Qt6::Widgets)
    endif()

    macro(dp_qt_add_resources)
        qt6_add_resources(${ARGN})
    endmacro()

    macro(dp_qt_wrap_ui)
        qt6_wrap_ui(${ARGN})
    endmacro()

	if(Qt6LinguistTools_FOUND)
        set(DP_QtLinguistTools_FOUND ON)
        macro(dp_qt_add_translation)
            qt6_add_translation(${ARGN})
        endmacro()
    else()
        unset(DP_QtLinguistTools_FOUND)
    endif()

else()
    message(FATAL_ERROR "Unknown QT_VERSION '${QT_VERSION}'")
endif()

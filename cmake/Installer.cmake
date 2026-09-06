# Qt Installer Framework packaging for the desktop QML app.
#
# Included from the top-level CMakeLists.txt when WISDOM_CHESS_INSTALLER is
# ON. Drives CPack's IFW generator from the install() rules of the
# WisdomChessQml target (see src/wisdom-chess/ui/qml/CMakeLists.txt, which
# also generates the Qt runtime deployment script). Produces:
#
#   Linux:   wisdom-chess-<version>-Linux-<arch>.run
#   Windows: wisdom-chess-<version>-Windows-<arch>.exe
#   macOS:   wisdom-chess-<version>-Darwin-<arch>.dmg
#
# Usage: cmake --build <build-dir> --target installer

get_property(_wisdom_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _wisdom_multi_config AND NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "-- wisdom-chess: set CMAKE_BUILD_TYPE (Release or RelWithDebInfo) when building the installer")
endif()

# --- Locate the Qt Installer Framework.
#
# CMake's CPackIFW module only searches Tools/QtInstallerFramework/<version>
# directories for versions it knew about when it was released (up to 4.5 in
# CMake 3.28), so look for the newest one ourselves unless the user already
# pointed us at one. IQTA_TOOLS is the Tools directory exported by
# jurplel/install-qt-action in CI.
if(NOT CPACK_IFW_ROOT AND "$ENV{CPACK_IFW_ROOT}" STREQUAL "" AND "$ENV{QTIFWDIR}" STREQUAL "")
    set(_wisdom_qt_tools_dirs "$ENV{IQTA_TOOLS}")
    if(WIN32)
        list(APPEND _wisdom_qt_tools_dirs
            "C:/Qt/Tools" "$ENV{HOMEDRIVE}/Qt/Tools" "$ENV{USERPROFILE}/Qt/Tools")
    else()
        list(APPEND _wisdom_qt_tools_dirs "$ENV{HOME}/Qt/Tools" "/opt/Qt/Tools")
    endif()
    foreach(_wisdom_tools_dir IN LISTS _wisdom_qt_tools_dirs)
        if(NOT _wisdom_tools_dir)
            continue()
        endif()
        file(GLOB _wisdom_ifw_dirs LIST_DIRECTORIES true
            "${_wisdom_tools_dir}/QtInstallerFramework/*")
        if(_wisdom_ifw_dirs)
            list(SORT _wisdom_ifw_dirs COMPARE NATURAL ORDER DESCENDING)
            list(GET _wisdom_ifw_dirs 0 CPACK_IFW_ROOT)
            break()
        endif()
    endforeach()
endif()

# --- Package identity
set(CPACK_GENERATOR "IFW")
set(CPACK_PACKAGE_NAME "WisdomChess")
set(CPACK_PACKAGE_VENDOR "Dave Meybohm")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Wisdom Chess")
# Becomes @ApplicationsDir@/Wisdom Chess on Windows and macOS.
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Wisdom Chess")
set(CPACK_PACKAGE_FILE_NAME
    "wisdom-chess-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
# Only the QML app component. Keeps the console binary and the React web
# build (installed under the "Unspecified" component) out of the installer.
set(CPACK_COMPONENTS_ALL Application)

# --- Installer look and behaviour
set(CPACK_IFW_PACKAGE_TITLE "Wisdom Chess")
set(CPACK_IFW_PACKAGE_PUBLISHER "${CPACK_PACKAGE_VENDOR}")
set(CPACK_IFW_PRODUCT_URL "https://github.com/dmeybohm/wisdom-chess")
set(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")
set(CPACK_IFW_PACKAGE_WINDOW_ICON "${CMAKE_SOURCE_DIR}/installer/icons/wisdom-chess-256.png")
set(CPACK_IFW_PACKAGE_LOGO "${CMAKE_SOURCE_DIR}/installer/icons/wisdom-chess-128.png")
set(CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME "WisdomChessMaintenanceTool")
set(CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS ON)
set(CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH ON)
set(CPACK_IFW_PACKAGE_RUN_PROGRAM_DESCRIPTION "Launch Wisdom Chess")

set(_wisdom_images_dir "${CMAKE_SOURCE_DIR}/src/wisdom-chess/ui/qml/images")
if(WIN32)
    set(CPACK_IFW_PACKAGE_ICON "${_wisdom_images_dir}/wisdom-chess.ico")
    set(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY "Wisdom Chess")
    set(CPACK_IFW_PACKAGE_RUN_PROGRAM "@TargetDir@/bin/WisdomChessQml.exe")
elseif(APPLE)
    # The .app lands in /Applications/Wisdom Chess/ alongside the
    # maintenance tool. Installing straight into /Applications would drop
    # the maintenance tool and components.xml there, and the installer
    # removes the target directory on uninstall.
    set(CPACK_IFW_PACKAGE_ICON "${_wisdom_images_dir}/wisdom-chess.icns")
    set(CPACK_IFW_PACKAGE_RUN_PROGRAM "@TargetDir@/WisdomChessQml.app/Contents/MacOS/WisdomChessQml")
else()
    set(CPACK_IFW_TARGET_DIRECTORY "/opt/WisdomChess")
    set(CPACK_IFW_ADMIN_TARGET_DIRECTORY "/opt/WisdomChess")
    set(CPACK_IFW_PACKAGE_RUN_PROGRAM "@TargetDir@/bin/WisdomChessQml")
endif()

include(CPackIFW)
if(NOT CPACK_IFW_BINARYCREATOR_EXECUTABLE)
    message(FATAL_ERROR "-- wisdom-chess: Qt Installer Framework not found. Install it with the Qt "
        "Maintenance Tool or pass -DCPACK_IFW_ROOT=<path to Tools/QtInstallerFramework/<version>>")
endif()
message(STATUS "wisdom-chess: using Qt Installer Framework ${CPACK_IFW_FRAMEWORK_VERSION} (${CPACK_IFW_BINARYCREATOR_EXECUTABLE})")

include(CPack)

cpack_add_component(Application
    DISPLAY_NAME "Wisdom Chess"
    DESCRIPTION "The Wisdom Chess desktop application."
    REQUIRED)
cpack_ifw_configure_component(Application
    NAME com.daveme.wisdomchess
    FORCED_INSTALLATION
    SCRIPT "${CMAKE_SOURCE_DIR}/installer/installscript.qs"
    LICENSES "MIT License" "${CMAKE_SOURCE_DIR}/LICENSE")

add_custom_target(installer
    COMMAND "${CMAKE_CPACK_COMMAND}" -G IFW -C $<CONFIG>
            --config "${CMAKE_BINARY_DIR}/CPackConfig.cmake"
    DEPENDS WisdomChessQml
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Building the Wisdom Chess installer with the Qt Installer Framework"
    USES_TERMINAL VERBATIM)

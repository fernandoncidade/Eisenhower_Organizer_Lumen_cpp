cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Lumen_Eisenhower_Mocks LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt6 6.12 REQUIRED COMPONENTS Core Gui Widgets)
endif()

if(WIN32)
    enable_language(RC)
endif()

find_package(Qt6 6.12 QUIET COMPONENTS LinguistTools)

if(WIN32)
    if(MSVC)
        set(MOCKS_DETECTED_TOOLCHAIN_SUFFIX "msvc")
    elseif(MINGW)
        set(MOCKS_DETECTED_TOOLCHAIN_SUFFIX "mingw")
    elseif(DEFINED EISENHOWER_BUILD_TOOLCHAIN_SUFFIX)
        set(MOCKS_DETECTED_TOOLCHAIN_SUFFIX "${EISENHOWER_BUILD_TOOLCHAIN_SUFFIX}")
    else()
        set(MOCKS_DETECTED_TOOLCHAIN_SUFFIX "windows")
    endif()

    set(MOCKS_DEFAULT_EXPORT_DIR "D:/MISCELANEAS/VSCode/Mocks/mocks_${MOCKS_DETECTED_TOOLCHAIN_SUFFIX}")
else()
    set(MOCKS_DEFAULT_EXPORT_DIR "")
endif()

if(WIN32)
    if(MSVC)
        set(MOCKS_WINDOWS_ICON_NAME "autismo.ico")
    else()
        set(MOCKS_WINDOWS_ICON_NAME "organizador.ico")
    endif()

    set(MOCKS_WINDOWS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icones/${MOCKS_WINDOWS_ICON_NAME}")
endif()

set(
    MOCKS_EXPORT_DIR
    "${MOCKS_DEFAULT_EXPORT_DIR}"
    CACHE PATH
    "Additional directory that receives the deployed mocks runtime"
)

if(MOCKS_EXPORT_DIR)
    file(TO_CMAKE_PATH "${MOCKS_EXPORT_DIR}" MOCKS_EXPORT_DIR_NORMALIZED)
else()
    set(MOCKS_EXPORT_DIR_NORMALIZED "")
endif()

function(mocks_add_windows_icon TARGET_NAME)
    if(NOT WIN32)
        return()
    endif()

    if(NOT EXISTS "${MOCKS_WINDOWS_ICON}")
        message(WARNING "Mock icon was not found at ${MOCKS_WINDOWS_ICON}. ${TARGET_NAME} will be built without the requested icon.")
        return()
    endif()

    file(TO_NATIVE_PATH "${MOCKS_WINDOWS_ICON}" EISENHOWER_APP_ICON_NATIVE)
    string(REPLACE "\\" "\\\\" EISENHOWER_APP_ICON_RC_PATH "${EISENHOWER_APP_ICON_NATIVE}")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../source/windows_app_icon.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_app_icon.rc"
        @ONLY
    )
    target_sources(
        "${TARGET_NAME}"
        PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_app_icon.rc"
    )

    target_compile_definitions(
        "${TARGET_NAME}"
        PRIVATE
            "MOCKS_WINDOWS_ICON_NAME=\"${MOCKS_WINDOWS_ICON_NAME}\""
    )

    add_custom_command(
        TARGET "${TARGET_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/icones"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${MOCKS_WINDOWS_ICON}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/icones/${MOCKS_WINDOWS_ICON_NAME}"
        VERBATIM
    )
endfunction()

function(mocks_apply_common_options TARGET_NAME)
    set_target_properties(
        "${TARGET_NAME}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    target_include_directories(
        "${TARGET_NAME}"
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )

    target_compile_definitions(
        "${TARGET_NAME}"
        PRIVATE
            $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
            $<$<CONFIG:Release>:QT_NO_INFO_OUTPUT>
            $<$<CONFIG:RelWithDebInfo>:QT_NO_DEBUG_OUTPUT>
            $<$<CONFIG:RelWithDebInfo>:QT_NO_INFO_OUTPUT>
    )

    if(COMMAND eisenhower_apply_performance_options)
        eisenhower_apply_performance_options("${TARGET_NAME}")
    elseif(MSVC)
        set_property(TARGET "${TARGET_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        target_compile_options("${TARGET_NAME}" PRIVATE /Zc:char8_t-)
        target_compile_options(
            "${TARGET_NAME}"
            PRIVATE
                $<$<CONFIG:Release>:/O2>
                $<$<CONFIG:Release>:/Oi>
                $<$<CONFIG:Release>:/Ot>
                $<$<CONFIG:Release>:/fp:fast>
                $<$<CONFIG:RelWithDebInfo>:/O2>
                $<$<CONFIG:RelWithDebInfo>:/Oi>
                $<$<CONFIG:RelWithDebInfo>:/Ot>
                $<$<CONFIG:RelWithDebInfo>:/fp:fast>
        )
    elseif(MINGW)
        target_compile_options("${TARGET_NAME}" PRIVATE -fno-char8_t)
        target_compile_options(
            "${TARGET_NAME}"
            PRIVATE
                $<$<CONFIG:Release>:-Ofast>
                $<$<CONFIG:Release>:-ffunction-sections>
                $<$<CONFIG:Release>:-fdata-sections>
                $<$<CONFIG:Release>:-fomit-frame-pointer>
                $<$<CONFIG:RelWithDebInfo>:-Ofast>
                $<$<CONFIG:RelWithDebInfo>:-ffunction-sections>
                $<$<CONFIG:RelWithDebInfo>:-fdata-sections>
                $<$<CONFIG:RelWithDebInfo>:-fomit-frame-pointer>
        )
        target_link_options(
            "${TARGET_NAME}"
            PRIVATE
                $<$<CONFIG:Release>:-Wl,--gc-sections>
                $<$<CONFIG:Release>:-Wl,-O1>
                $<$<CONFIG:RelWithDebInfo>:-Wl,--gc-sections>
                $<$<CONFIG:RelWithDebInfo>:-Wl,-O1>
        )
    endif()
endfunction()

function(mocks_add_deploy_target TARGET_NAME)
    if(NOT WIN32)
        return()
    endif()

    get_filename_component(MOCKS_QT_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
    set(MOCKS_WINDEPLOYQT_EXECUTABLE "${MOCKS_QT_PREFIX_DIR}/bin/windeployqt.exe")
    set(MOCKS_QTPATHS_EXECUTABLE "${MOCKS_QT_PREFIX_DIR}/bin/qtpaths6.exe")
    if(NOT EXISTS "${MOCKS_WINDEPLOYQT_EXECUTABLE}" OR NOT EXISTS "${MOCKS_QTPATHS_EXECUTABLE}")
        message(WARNING "windeployqt.exe or qtpaths6.exe was not found. ${TARGET_NAME} will not be deployed automatically.")
        return()
    endif()

    set(MOCKS_DEPLOY_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/deploy_optional_qt_binary.cmake")
    set(MOCKS_NO_COMPILER_RUNTIME OFF)
    if(MSVC)
        set(MOCKS_NO_COMPILER_RUNTIME ON)
    endif()
    set(MOCKS_QT_TRANSLATIONS "pt_BR,en")
    if(DEFINED EISENHOWER_QT_DEPLOY_TRANSLATIONS)
        set(MOCKS_QT_TRANSLATIONS "${EISENHOWER_QT_DEPLOY_TRANSLATIONS}")
    endif()
    set(MOCKS_MINGW_BIN_DIR "")
    if(MINGW)
        if(DEFINED EISENHOWER_MINGW_BIN_DIR AND NOT "${EISENHOWER_MINGW_BIN_DIR}" STREQUAL "")
            set(MOCKS_MINGW_BIN_DIR "${EISENHOWER_MINGW_BIN_DIR}")
        else()
            get_filename_component(MOCKS_MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        endif()
    endif()
    set(MOCKS_MSVC_REDIST_ROOT "")
    if(DEFINED EISENHOWER_MSVC_REDIST_ROOT)
        set(MOCKS_MSVC_REDIST_ROOT "${EISENHOWER_MSVC_REDIST_ROOT}")
    elseif(DEFINED ENV{VCToolsRedistDir} AND EXISTS "$ENV{VCToolsRedistDir}")
        set(MOCKS_MSVC_REDIST_ROOT "$ENV{VCToolsRedistDir}")
    endif()
    set(MOCKS_MSVC_RUNTIME_ARCH "x64")
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(MOCKS_MSVC_RUNTIME_ARCH "x86")
    endif()

    set(MOCKS_BUILD_RUNTIME_COMMANDS)
    set(MOCKS_EXPORT_RUNTIME_COMMANDS)
    if(MSVC)
        list(APPEND MOCKS_BUILD_RUNTIME_COMMANDS
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                "-DMSVC_REDIST_ROOT=${MOCKS_MSVC_REDIST_ROOT}"
                "-DMSVC_RUNTIME_ARCH=${MOCKS_MSVC_RUNTIME_ARCH}"
                "-DCONFIG=$<CONFIG>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/copy_msvc_runtime.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                "-DPRUNE_MSVC_RUNTIME=OFF"
                "-DPRUNE_MINGW_RUNTIME=ON"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prune_optional_root_runtime_dlls.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prune_optional_qt_sqldrivers.cmake"
        )
        list(APPEND MOCKS_EXPORT_RUNTIME_COMMANDS
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=${MOCKS_EXPORT_DIR_NORMALIZED}"
                "-DMSVC_REDIST_ROOT=${MOCKS_MSVC_REDIST_ROOT}"
                "-DMSVC_RUNTIME_ARCH=${MOCKS_MSVC_RUNTIME_ARCH}"
                "-DCONFIG=$<CONFIG>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/copy_msvc_runtime.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=${MOCKS_EXPORT_DIR_NORMALIZED}"
                "-DPRUNE_MSVC_RUNTIME=OFF"
                "-DPRUNE_MINGW_RUNTIME=ON"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prune_optional_root_runtime_dlls.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=${MOCKS_EXPORT_DIR_NORMALIZED}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prune_optional_qt_sqldrivers.cmake"
        )
    elseif(MINGW)
        list(APPEND MOCKS_BUILD_RUNTIME_COMMANDS
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                "-DMINGW_BIN_DIR=${MOCKS_MINGW_BIN_DIR}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/copy_mingw_runtime.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                "-DPRUNE_MSVC_RUNTIME=ON"
                "-DPRUNE_MINGW_RUNTIME=OFF"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prune_optional_root_runtime_dlls.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prune_optional_qt_sqldrivers.cmake"
        )
        list(APPEND MOCKS_EXPORT_RUNTIME_COMMANDS
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=${MOCKS_EXPORT_DIR_NORMALIZED}"
                "-DMINGW_BIN_DIR=${MOCKS_MINGW_BIN_DIR}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/copy_mingw_runtime.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=${MOCKS_EXPORT_DIR_NORMALIZED}"
                "-DPRUNE_MSVC_RUNTIME=ON"
                "-DPRUNE_MINGW_RUNTIME=OFF"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prune_optional_root_runtime_dlls.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=${MOCKS_EXPORT_DIR_NORMALIZED}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prune_optional_qt_sqldrivers.cmake"
        )
    endif()

    add_custom_target(
        "${TARGET_NAME}Deploy"
        ALL
        DEPENDS "${TARGET_NAME}"
        COMMAND "${CMAKE_COMMAND}"
            "-DWINDEPLOYQT_EXECUTABLE=${MOCKS_WINDEPLOYQT_EXECUTABLE}"
            "-DQTPATHS_EXECUTABLE=${MOCKS_QTPATHS_EXECUTABLE}"
            "-DTARGET_FILE=$<TARGET_FILE:${TARGET_NAME}>"
            "-DRUNTIME_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>"
            "-DPLUGIN_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>/plugins"
            "-DTRANSLATION_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>/translations"
            "-DQT_TRANSLATIONS=${MOCKS_QT_TRANSLATIONS}"
            "-DNO_COMPILER_RUNTIME=${MOCKS_NO_COMPILER_RUNTIME}"
            -P "${MOCKS_DEPLOY_SCRIPT}"
        ${MOCKS_BUILD_RUNTIME_COMMANDS}
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${MOCKS_EXPORT_DIR_NORMALIZED}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${MOCKS_EXPORT_DIR_NORMALIZED}/icones"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${MOCKS_WINDOWS_ICON}"
            "${MOCKS_EXPORT_DIR_NORMALIZED}/icones/${MOCKS_WINDOWS_ICON_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:${TARGET_NAME}>"
            "${MOCKS_EXPORT_DIR_NORMALIZED}/"
        COMMAND "${CMAKE_COMMAND}"
            "-DWINDEPLOYQT_EXECUTABLE=${MOCKS_WINDEPLOYQT_EXECUTABLE}"
            "-DQTPATHS_EXECUTABLE=${MOCKS_QTPATHS_EXECUTABLE}"
            "-DTARGET_FILE=${MOCKS_EXPORT_DIR_NORMALIZED}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
            "-DRUNTIME_DIR=${MOCKS_EXPORT_DIR_NORMALIZED}"
            "-DPLUGIN_DIR=${MOCKS_EXPORT_DIR_NORMALIZED}/plugins"
            "-DTRANSLATION_DIR=${MOCKS_EXPORT_DIR_NORMALIZED}/translations"
            "-DQT_TRANSLATIONS=${MOCKS_QT_TRANSLATIONS}"
            "-DNO_COMPILER_RUNTIME=${MOCKS_NO_COMPILER_RUNTIME}"
            -P "${MOCKS_DEPLOY_SCRIPT}"
        ${MOCKS_EXPORT_RUNTIME_COMMANDS}
        COMMENT "Deploying ${TARGET_NAME} runtime"
        VERBATIM
    )

    if(COMMAND eisenhower_chain_deploy_target)
        eisenhower_chain_deploy_target("${TARGET_NAME}Deploy")
    elseif(MOCKS_LAST_DEPLOY_TARGET)
        add_dependencies("${TARGET_NAME}Deploy" "${MOCKS_LAST_DEPLOY_TARGET}")
    endif()
    set(MOCKS_LAST_DEPLOY_TARGET "${TARGET_NAME}Deploy" PARENT_SCOPE)
endfunction()

function(mocks_add_lrelease_target TARGET_NAME)
    if(NOT TARGET Qt6::lrelease)
        message(WARNING "Qt6 LinguistTools/lrelease was not found. ${TARGET_NAME} will not be available.")
        return()
    endif()

    file(
        GLOB
        MOCKS_TS_FILES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/../source/language/translations/*.ts"
    )
    set(MOCKS_QM_OUTPUTS)

    foreach(MOCKS_TS_FILE IN LISTS MOCKS_TS_FILES)
        get_filename_component(MOCKS_TS_BASENAME "${MOCKS_TS_FILE}" NAME_WE)
        get_filename_component(MOCKS_TS_DIR "${MOCKS_TS_FILE}" DIRECTORY)
        set(MOCKS_QM_FILE "${MOCKS_TS_DIR}/${MOCKS_TS_BASENAME}.qm")
        list(APPEND MOCKS_QM_OUTPUTS "${MOCKS_QM_FILE}")
    endforeach()

    if(MOCKS_TS_FILES)
        add_custom_command(
            OUTPUT ${MOCKS_QM_OUTPUTS}
            COMMAND Qt6::lrelease
                ${MOCKS_TS_FILES}
            DEPENDS ${MOCKS_TS_FILES}
            COMMENT "Compilando todos os arquivos de tradução .ts em .qm em conjunto"
            VERBATIM
        )
        add_custom_target("${TARGET_NAME}" DEPENDS ${MOCKS_QM_OUTPUTS})
    endif()
endfunction()

function(mocks_add_gui_run_target TARGET_NAME TOOL_TARGET)
    if(NOT WIN32)
        return()
    endif()

    add_custom_target(
        "${TARGET_NAME}"
        DEPENDS "${TOOL_TARGET}Deploy"
        COMMAND "${CMAKE_COMMAND}" -E env
            "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:${TOOL_TARGET}>"
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:${TOOL_TARGET}>/platforms"
            cmd /c start "" /b "$<TARGET_FILE:${TOOL_TARGET}>" ${ARGN}
        VERBATIM
        USES_TERMINAL
    )
endfunction()

function(mocks_add_terminal_run_target TARGET_NAME TOOL_TARGET)
    if(NOT WIN32)
        return()
    endif()

    add_custom_target(
        "${TARGET_NAME}"
        DEPENDS "${TOOL_TARGET}Deploy"
        COMMAND "${CMAKE_COMMAND}" -E chdir "$<TARGET_FILE_DIR:${TOOL_TARGET}>"
            "${CMAKE_COMMAND}" -E env
            "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:${TOOL_TARGET}>"
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:${TOOL_TARGET}>/platforms"
            "$<TARGET_FILE:${TOOL_TARGET}>"
            ${ARGN}
        VERBATIM
        USES_TERMINAL
    )
endfunction()

add_executable(
    CompileTranslationsGUI
    WIN32
    CompileTranslationsGUI.cpp
    CompileTranslationsGUI.hpp
)
target_link_libraries(CompileTranslationsGUI PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
mocks_add_windows_icon(CompileTranslationsGUI)
mocks_apply_common_options(CompileTranslationsGUI)
mocks_add_deploy_target(CompileTranslationsGUI)

add_executable(
    CompileTranslationsTerminal
    CompileTranslationsTerminal.cpp
    CompileTranslationsTerminal.hpp
)
target_link_libraries(CompileTranslationsTerminal PRIVATE Qt6::Core)
if(WIN32)
    target_link_libraries(CompileTranslationsTerminal PRIVATE User32)
endif()
mocks_add_windows_icon(CompileTranslationsTerminal)
mocks_apply_common_options(CompileTranslationsTerminal)
mocks_add_deploy_target(CompileTranslationsTerminal)

mocks_add_lrelease_target(CompileTranslationsQm)

add_executable(
    find_VersionEditor
    WIN32
    find_VersionEditor.cpp
    find_VersionEditor.hpp
)
target_link_libraries(find_VersionEditor PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_compile_features(find_VersionEditor PRIVATE cxx_std_23)
mocks_add_windows_icon(find_VersionEditor)
mocks_apply_common_options(find_VersionEditor)
mocks_add_deploy_target(find_VersionEditor)

if(WIN32)
    set(MOCKS_DETACHED_GUI_LAUNCHER "${CMAKE_CURRENT_SOURCE_DIR}/launch_detached_gui.cmake")

    add_custom_target(
        MocksRun
        DEPENDS
            CompileTranslationsGUIDeploy
            CompileTranslationsTerminalDeploy
            find_VersionEditorDeploy
        COMMAND "${CMAKE_COMMAND}" -E env
            "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:find_VersionEditor>"
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:find_VersionEditor>/platforms"
            "${CMAKE_COMMAND}"
            -DLAUNCH_FILE=$<TARGET_FILE:find_VersionEditor>
            -DLAUNCH_WORKING_DIR=$<TARGET_FILE_DIR:find_VersionEditor>
            -P "${MOCKS_DETACHED_GUI_LAUNCHER}"
        COMMAND "${CMAKE_COMMAND}" -E env
            "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:CompileTranslationsGUI>"
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:CompileTranslationsGUI>/platforms"
            "${CMAKE_COMMAND}"
            -DLAUNCH_FILE=$<TARGET_FILE:CompileTranslationsGUI>
            -DLAUNCH_WORKING_DIR=$<TARGET_FILE_DIR:CompileTranslationsGUI>
            -DLAUNCH_ARGS=--all
            -P "${MOCKS_DETACHED_GUI_LAUNCHER}"
        COMMENT "Launching mocks runtime tools"
        VERBATIM
        USES_TERMINAL
    )

    mocks_add_gui_run_target(find_VersionEditorRun find_VersionEditor)
    mocks_add_gui_run_target(CompileTranslationsGUIRun CompileTranslationsGUI --all)
    mocks_add_gui_run_target(CompileTranslationsGUIRunAll CompileTranslationsGUI --all)
    mocks_add_gui_run_target(CompileTranslationsGUIRunEisenhower CompileTranslationsGUI --module eisenhower)
    mocks_add_gui_run_target(CompileTranslationsGUIRunPomodoro CompileTranslationsGUI --module pomodoro)
    mocks_add_gui_run_target(CompileTranslationsGUIRunFeynman CompileTranslationsGUI --module feynman)
    mocks_add_gui_run_target(CompileTranslationsGUIRunMapaMental CompileTranslationsGUI --module mapamental)
    mocks_add_gui_run_target(CompileTranslationsGUIRunAudiolivro CompileTranslationsGUI --module audiolivro)
    mocks_add_gui_run_target(CompileTranslationsGUIRunAgenda CompileTranslationsGUI --module agenda)
    mocks_add_gui_run_target(CompileTranslationsGUIRunGantt CompileTranslationsGUI --module gantt)

    mocks_add_terminal_run_target(CompileTranslationsTerminalRun CompileTranslationsTerminal --all)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunAll CompileTranslationsTerminal --all)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunEisenhower CompileTranslationsTerminal --module eisenhower)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunPomodoro CompileTranslationsTerminal --module pomodoro)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunFeynman CompileTranslationsTerminal --module feynman)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunMapaMental CompileTranslationsTerminal --module mapamental)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunAudiolivro CompileTranslationsTerminal --module audiolivro)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunAgenda CompileTranslationsTerminal --module agenda)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunGantt CompileTranslationsTerminal --module gantt)
endif()

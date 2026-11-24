cmake_minimum_required(VERSION 3.28)

project(Lumen_Eisenhower_Organizer_cpp LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckIPOSupported)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()
if(POLICY CMP0138)
    cmake_policy(SET CMP0138 NEW)
endif()

option(EISENHOWER_BUILD_HOST_APP "Build the Lumen/Eisenhower desktop application" ON)
option(EISENHOWER_MODULAR_PLUGINS "Build lean host executable that dynamically loads the 7 standalone plugin DLLs via QPluginLoader" OFF)
option(EISENHOWER_BUILD_FEYNMAN_PLUGIN "Build the Feynman plugin DLL" OFF)
option(EISENHOWER_BUILD_POMODORO_PLUGIN "Build the Pomodoro plugin DLL" OFF)
option(EISENHOWER_BUILD_MAPAMENTAL_PLUGIN "Build the Mapa Mental plugin DLL" OFF)
option(EISENHOWER_BUILD_MODELO_PLUGIN "Build the optional Qwen model service plugin DLL" OFF)
option(EISENHOWER_BUILD_AUDIOLIVRO_PLUGIN "Build the Audiolivro plugin DLL" OFF)
option(EISENHOWER_BUILD_AGENDA_PLUGIN "Build the Agenda de Avaliações plugin DLL" OFF)
option(EISENHOWER_BUILD_GANTT_PLUGIN "Build the Gantt plugin DLL" OFF)
option(EISENHOWER_BUILD_MOCKS "Build development and translation utility tools from mocks" OFF)
option(EISENHOWER_BUILD_TESTS "Build Qt Test based unit tests" OFF)
option(EISENHOWER_ENABLE_IPO "Enable interprocedural/link-time optimization for optimized builds" OFF)
option(EISENHOWER_ENABLE_NATIVE_OPTIMIZATIONS "Optimize Release binaries for this machine's CPU instruction set" OFF)
option(EISENHOWER_ENABLE_FAST_MATH "Allow aggressive floating-point optimizations in optimized builds" OFF)
option(EISENHOWER_ENABLE_AGGRESSIVE_INLINING "Enable additional favor-speed and inlining optimizations" OFF)
option(EISENHOWER_STRIP_RELEASE_BINARIES "Strip MinGW Release binaries after linking" ON)
option(EISENHOWER_DEPLOY_STANDALONE_PLUGIN_RUNTIME "Copy Qt/runtime DLLs into individual plugin build trees" OFF)
option(EISENHOWER_PRUNE_OPTIONAL_ROOT_RUNTIME_DLLS "Remove optional root runtime DLLs that are not imported by the deployed products" ON)
option(EISENHOWER_EMBED_GGUF_MODEL "Embed the 2.1GB GGUF model in the binary package" OFF)

set(EISENHOWER_QT_DEPLOY_TRANSLATIONS "pt_BR,en" CACHE STRING "Comma-separated Qt translation languages deployed by windeployqt")

set(BUILD_TARGET "EISENHOWER" CACHE STRING "Application build target: EISENHOWER or LUMEN")
set_property(CACHE BUILD_TARGET PROPERTY STRINGS EISENHOWER LUMEN)
string(TOUPPER "${BUILD_TARGET}" BUILD_TARGET)
if(NOT BUILD_TARGET STREQUAL "EISENHOWER" AND NOT BUILD_TARGET STREQUAL "LUMEN")
    message(FATAL_ERROR "BUILD_TARGET must be EISENHOWER or LUMEN.")
endif()

if(BUILD_TARGET STREQUAL "LUMEN")
    set(APP_ASSETS_SUBDIR "lumen")
    set(APP_WINDOWS_ICON_NAME "autismo.ico")
    set(APP_EXECUTABLE_NAME "Lumen")
    set(APP_EXPORT_DIR_BASENAME "Lumen_cpp")
    set(APP_EXPORT_ROOT_DIR "D:/MISCELANEAS/VSCode/Lumen")
else()
    set(APP_ASSETS_SUBDIR "eisenhower")
    set(APP_WINDOWS_ICON_NAME "organizador.ico")
    set(APP_EXECUTABLE_NAME "EisenhowerOrganizer")
    set(APP_EXPORT_DIR_BASENAME "Eisenhower_Organizer_cpp")
    set(APP_EXPORT_ROOT_DIR "D:/MISCELANEAS/VSCode/EisenhowerOrganizer")
endif()

set(EISENHOWER_MSVC_ARCH "OFF" CACHE STRING "MSVC /arch value for native optimized builds: OFF, AVX, AVX2, or AVX512")
set_property(CACHE EISENHOWER_MSVC_ARCH PROPERTY STRINGS OFF AVX AVX2 AVX512)
set(EISENHOWER_GNU_ARCH "x86-64" CACHE STRING "GCC/MinGW -march value used when native optimizations are enabled")
set(EISENHOWER_GNU_TUNE "generic" CACHE STRING "GCC/MinGW -mtune value used when native optimizations are enabled")

set(EISENHOWER_PGO_MODE "OFF" CACHE STRING "Profile-guided optimization mode: OFF, GENERATE, or USE")
set_property(CACHE EISENHOWER_PGO_MODE PROPERTY STRINGS OFF GENERATE USE)
string(TOUPPER "${EISENHOWER_PGO_MODE}" EISENHOWER_PGO_MODE)
if(NOT EISENHOWER_PGO_MODE STREQUAL "OFF"
   AND NOT EISENHOWER_PGO_MODE STREQUAL "GENERATE"
   AND NOT EISENHOWER_PGO_MODE STREQUAL "USE")
    message(FATAL_ERROR "EISENHOWER_PGO_MODE must be OFF, GENERATE, or USE.")
endif()

set(EISENHOWER_PGO_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/pgo" CACHE PATH "Directory used by PGO generate/use builds")
set(EISENHOWER_MSVC_ARCH_VALUE "${EISENHOWER_MSVC_ARCH}")
string(TOUPPER "${EISENHOWER_MSVC_ARCH_VALUE}" EISENHOWER_MSVC_ARCH_VALUE)
if(NOT EISENHOWER_MSVC_ARCH_VALUE STREQUAL "OFF"
   AND NOT EISENHOWER_MSVC_ARCH_VALUE STREQUAL "AVX"
   AND NOT EISENHOWER_MSVC_ARCH_VALUE STREQUAL "AVX2"
   AND NOT EISENHOWER_MSVC_ARCH_VALUE STREQUAL "AVX512")
    message(FATAL_ERROR "EISENHOWER_MSVC_ARCH must be OFF, AVX, AVX2, or AVX512.")
endif()

set(EISENHOWER_BUILD_TOOLCHAIN_SUFFIX "generic" CACHE STRING "Suffix used for toolchain-specific build directories")

if(WIN32)
    enable_language(RC)

    if(MSVC)
        set(EISENHOWER_DETECTED_TOOLCHAIN_SUFFIX "msvc")
    elseif(MINGW)
        set(EISENHOWER_DETECTED_TOOLCHAIN_SUFFIX "mingw")
    else()
        set(EISENHOWER_DETECTED_TOOLCHAIN_SUFFIX "windows")
    endif()

    if(EISENHOWER_BUILD_TOOLCHAIN_SUFFIX STREQUAL "generic")
        set(EISENHOWER_BUILD_TOOLCHAIN_SUFFIX "${EISENHOWER_DETECTED_TOOLCHAIN_SUFFIX}" CACHE STRING "Suffix used for toolchain-specific build directories" FORCE)
    endif()
endif()

set(EISENHOWER_DEFAULT_EXPORT_DIR "${APP_EXPORT_ROOT_DIR}/${APP_EXPORT_DIR_BASENAME}")
if(WIN32)
    if(EISENHOWER_BUILD_TOOLCHAIN_SUFFIX MATCHES "^msvc")
        set(EISENHOWER_DEFAULT_EXPORT_DIR "${APP_EXPORT_ROOT_DIR}/${APP_EXPORT_DIR_BASENAME}_msvc")
    elseif(EISENHOWER_BUILD_TOOLCHAIN_SUFFIX STREQUAL "mingw")
        set(EISENHOWER_DEFAULT_EXPORT_DIR "${APP_EXPORT_ROOT_DIR}/${APP_EXPORT_DIR_BASENAME}_mingw")
    endif()
endif()

set(APP_PLUGIN_BUILD_SUFFIX "${EISENHOWER_BUILD_TOOLCHAIN_SUFFIX}")

set(
    EISENHOWER_EXPORT_DIR
    "${EISENHOWER_DEFAULT_EXPORT_DIR}"
    CACHE PATH
    "Additional directory that receives the deployed application runtime"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_autogen_state.cmake")

set(EISENHOWER_MINGW_BIN_DIR "")
if(MINGW)
    get_filename_component(EISENHOWER_MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
endif()

if(EISENHOWER_QT_DEPLOY_TRANSLATIONS)
    set(EISENHOWER_WINDEPLOYQT_TRANSLATION_ARGS --translations "${EISENHOWER_QT_DEPLOY_TRANSLATIONS}")
else()
    set(EISENHOWER_WINDEPLOYQT_TRANSLATION_ARGS --no-translations)
endif()

set(EISENHOWER_WINDEPLOYQT_COMPILER_RUNTIME_ARGS)
set(EISENHOWER_WINDEPLOYQT_NO_COMPILER_RUNTIME OFF)
if(MSVC)
    set(EISENHOWER_WINDEPLOYQT_COMPILER_RUNTIME_ARGS --no-compiler-runtime)
    set(EISENHOWER_WINDEPLOYQT_NO_COMPILER_RUNTIME ON)
endif()

set(EISENHOWER_MSVC_RUNTIME_ARCH "")
set(EISENHOWER_MSVC_REDIST_ROOT "")
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(EISENHOWER_MSVC_RUNTIME_ARCH "x64")
    else()
        set(EISENHOWER_MSVC_RUNTIME_ARCH "x86")
    endif()

    if(DEFINED ENV{VCToolsRedistDir} AND EXISTS "$ENV{VCToolsRedistDir}")
        set(EISENHOWER_MSVC_REDIST_ROOT "$ENV{VCToolsRedistDir}")
    elseif(DEFINED ENV{VCINSTALLDIR} AND EXISTS "$ENV{VCINSTALLDIR}/Redist/MSVC")
        set(EISENHOWER_MSVC_REDIST_ROOT "$ENV{VCINSTALLDIR}/Redist/MSVC")
    elseif(CMAKE_CXX_COMPILER MATCHES "/VC/Tools/MSVC/")
        get_filename_component(EISENHOWER_MSVC_COMPILER_ARCH_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        get_filename_component(EISENHOWER_MSVC_COMPILER_HOST_DIR "${EISENHOWER_MSVC_COMPILER_ARCH_DIR}" DIRECTORY)
        get_filename_component(EISENHOWER_MSVC_COMPILER_BIN_DIR "${EISENHOWER_MSVC_COMPILER_HOST_DIR}" DIRECTORY)
        get_filename_component(EISENHOWER_MSVC_VERSION_DIR "${EISENHOWER_MSVC_COMPILER_BIN_DIR}" DIRECTORY)
        get_filename_component(EISENHOWER_MSVC_TOOLS_MSVC_DIR "${EISENHOWER_MSVC_VERSION_DIR}" DIRECTORY)
        get_filename_component(EISENHOWER_MSVC_TOOLS_DIR "${EISENHOWER_MSVC_TOOLS_MSVC_DIR}" DIRECTORY)
        get_filename_component(EISENHOWER_MSVC_VC_DIR "${EISENHOWER_MSVC_TOOLS_DIR}" DIRECTORY)
        if(EXISTS "${EISENHOWER_MSVC_VC_DIR}/Redist/MSVC")
            set(EISENHOWER_MSVC_REDIST_ROOT "${EISENHOWER_MSVC_VC_DIR}/Redist/MSVC")
        endif()
    endif()

    if(EISENHOWER_MSVC_REDIST_ROOT STREQUAL "")
        file(GLOB EISENHOWER_MSVC_REDIST_ROOT_CANDIDATES LIST_DIRECTORIES true
            "C:/Program Files/Microsoft Visual Studio/*/*/VC/Redist/MSVC"
            "C:/Program Files (x86)/Microsoft Visual Studio/*/*/VC/Redist/MSVC"
        )
        if(EISENHOWER_MSVC_REDIST_ROOT_CANDIDATES)
            list(SORT EISENHOWER_MSVC_REDIST_ROOT_CANDIDATES COMPARE NATURAL ORDER DESCENDING)
            list(GET EISENHOWER_MSVC_REDIST_ROOT_CANDIDATES 0 EISENHOWER_MSVC_REDIST_ROOT)
        endif()
    endif()
endif()

if(MSVC AND EISENHOWER_ENABLE_AGGRESSIVE_INLINING)
    foreach(EISENHOWER_MSVC_CONFIG RELEASE RELWITHDEBINFO)
        set(EISENHOWER_MSVC_FLAGS_VAR "CMAKE_CXX_FLAGS_${EISENHOWER_MSVC_CONFIG}")
        if(DEFINED ${EISENHOWER_MSVC_FLAGS_VAR})
            string(
                REGEX REPLACE
                "(^|[ \t]+)/Ob[0-2]($|[ \t])"
                " "
                ${EISENHOWER_MSVC_FLAGS_VAR}
                "${${EISENHOWER_MSVC_FLAGS_VAR}}"
            )
            string(STRIP "${${EISENHOWER_MSVC_FLAGS_VAR}}" ${EISENHOWER_MSVC_FLAGS_VAR})
        endif()
    endforeach()
endif()

if(WIN32)
    set(EISENHOWER_SELECTED_QT_CMAKE_DIR "")
    set(EISENHOWER_QT_CMAKE_CANDIDATE_DIRS "")

    foreach(EISENHOWER_QT_DIR_VAR Qt6_DIR QT_DIR)
        if(DEFINED ${EISENHOWER_QT_DIR_VAR})
            get_filename_component(EISENHOWER_QT_DIR_VALUE "${${EISENHOWER_QT_DIR_VAR}}" ABSOLUTE)
            if(EXISTS "${EISENHOWER_QT_DIR_VALUE}/Qt6Config.cmake")
                get_filename_component(EISENHOWER_QT_CMAKE_DIR_FROM_VAR "${EISENHOWER_QT_DIR_VALUE}" DIRECTORY)
                list(APPEND EISENHOWER_QT_CMAKE_CANDIDATE_DIRS "${EISENHOWER_QT_CMAKE_DIR_FROM_VAR}")
            elseif(EXISTS "${EISENHOWER_QT_DIR_VALUE}/Qt6/Qt6Config.cmake")
                list(APPEND EISENHOWER_QT_CMAKE_CANDIDATE_DIRS "${EISENHOWER_QT_DIR_VALUE}")
            endif()
        endif()
    endforeach()

    foreach(EISENHOWER_PREFIX_PATH IN LISTS CMAKE_PREFIX_PATH)
        get_filename_component(EISENHOWER_PREFIX_PATH_ABSOLUTE "${EISENHOWER_PREFIX_PATH}" ABSOLUTE)
        if(EXISTS "${EISENHOWER_PREFIX_PATH_ABSOLUTE}/Qt6/Qt6Config.cmake")
            list(APPEND EISENHOWER_QT_CMAKE_CANDIDATE_DIRS "${EISENHOWER_PREFIX_PATH_ABSOLUTE}")
        elseif(EXISTS "${EISENHOWER_PREFIX_PATH_ABSOLUTE}/lib/cmake/Qt6/Qt6Config.cmake")
            list(APPEND EISENHOWER_QT_CMAKE_CANDIDATE_DIRS "${EISENHOWER_PREFIX_PATH_ABSOLUTE}/lib/cmake")
        elseif(EXISTS "${EISENHOWER_PREFIX_PATH_ABSOLUTE}/Qt6Config.cmake")
            get_filename_component(EISENHOWER_QT_CMAKE_DIR_FROM_PREFIX "${EISENHOWER_PREFIX_PATH_ABSOLUTE}" DIRECTORY)
            list(APPEND EISENHOWER_QT_CMAKE_CANDIDATE_DIRS "${EISENHOWER_QT_CMAKE_DIR_FROM_PREFIX}")
        endif()
    endforeach()

    set(EISENHOWER_QT_COMPILER_DIR "")
    if(MSVC)
        set(EISENHOWER_QT_COMPILER_DIR "msvc2022_64")
    elseif(MINGW)
        set(EISENHOWER_QT_COMPILER_DIR "mingw_64")
    endif()

    if(EISENHOWER_QT_COMPILER_DIR)
        file(GLOB EISENHOWER_QT_CONFIG_CANDIDATES "C:/Qt/*/${EISENHOWER_QT_COMPILER_DIR}/lib/cmake/Qt6/Qt6Config.cmake")
        list(SORT EISENHOWER_QT_CONFIG_CANDIDATES COMPARE NATURAL ORDER DESCENDING)
        foreach(EISENHOWER_QT_CONFIG_CANDIDATE IN LISTS EISENHOWER_QT_CONFIG_CANDIDATES)
            get_filename_component(EISENHOWER_QT6_CONFIG_DIR "${EISENHOWER_QT_CONFIG_CANDIDATE}" DIRECTORY)
            get_filename_component(EISENHOWER_QT_CMAKE_DIR "${EISENHOWER_QT6_CONFIG_DIR}" DIRECTORY)
            list(APPEND EISENHOWER_QT_CMAKE_CANDIDATE_DIRS "${EISENHOWER_QT_CMAKE_DIR}")
        endforeach()
    endif()

    list(REMOVE_DUPLICATES EISENHOWER_QT_CMAKE_CANDIDATE_DIRS)
    foreach(EISENHOWER_QT_CMAKE_DIR_CANDIDATE IN LISTS EISENHOWER_QT_CMAKE_CANDIDATE_DIRS)
        if(EXISTS "${EISENHOWER_QT_CMAKE_DIR_CANDIDATE}/Qt6/Qt6Config.cmake")
            set(EISENHOWER_SELECTED_QT_CMAKE_DIR "${EISENHOWER_QT_CMAKE_DIR_CANDIDATE}")
            break()
        endif()
    endforeach()

    if(EISENHOWER_SELECTED_QT_CMAKE_DIR)
        get_cmake_property(EISENHOWER_CACHE_VARIABLES CACHE_VARIABLES)
        foreach(EISENHOWER_CACHE_VAR IN LISTS EISENHOWER_CACHE_VARIABLES)
            if(EISENHOWER_CACHE_VAR MATCHES "^Qt6.*_DIR$" OR EISENHOWER_CACHE_VAR STREQUAL "Qt6_DIR" OR EISENHOWER_CACHE_VAR STREQUAL "QT_DIR")
                unset(${EISENHOWER_CACHE_VAR} CACHE)
            endif()
        endforeach()

        list(PREPEND CMAKE_PREFIX_PATH "${EISENHOWER_SELECTED_QT_CMAKE_DIR}")
        set(QT_DIR "${EISENHOWER_SELECTED_QT_CMAKE_DIR}/Qt6" CACHE PATH "Qt6 package directory" FORCE)
        set(Qt6_DIR "${EISENHOWER_SELECTED_QT_CMAKE_DIR}/Qt6" CACHE PATH "Qt6 package directory" FORCE)

        foreach(EISENHOWER_QT_COMPONENT Core CorePrivate Gui Widgets Multimedia Network OpenGLWidgets Pdf PdfWidgets Sql WebSockets Test LinguistTools)
            if(EXISTS "${EISENHOWER_SELECTED_QT_CMAKE_DIR}/Qt6${EISENHOWER_QT_COMPONENT}/Qt6${EISENHOWER_QT_COMPONENT}Config.cmake")
                set(
                    Qt6${EISENHOWER_QT_COMPONENT}_DIR
                    "${EISENHOWER_SELECTED_QT_CMAKE_DIR}/Qt6${EISENHOWER_QT_COMPONENT}"
                    CACHE PATH "Qt6${EISENHOWER_QT_COMPONENT} package directory" FORCE
                )
            endif()
        endforeach()
    endif()
endif()

find_package(Qt6 6.12 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
    Network
    OpenGLWidgets
    Pdf
    PdfWidgets
    Sql
    WebSockets
)
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6CorePrivate 6.12 REQUIRED)

if(EISENHOWER_BUILD_TESTS)
    enable_testing()
    find_package(Qt6 6.12 REQUIRED COMPONENTS Test)
endif()

function(eisenhower_get_qt_target_location TARGET_NAME OUT_VAR)
    set(EISENHOWER_QT_LOCATION "")
    if(TARGET "${TARGET_NAME}")
        foreach(EISENHOWER_LOCATION_PROP IN ITEMS
                IMPORTED_LOCATION_RELEASE
                IMPORTED_LOCATION_RELWITHDEBINFO
                IMPORTED_LOCATION_DEBUG
                IMPORTED_LOCATION)
            get_target_property(EISENHOWER_CANDIDATE_LOCATION "${TARGET_NAME}" "${EISENHOWER_LOCATION_PROP}")
            if(EISENHOWER_CANDIDATE_LOCATION AND NOT EISENHOWER_CANDIDATE_LOCATION STREQUAL "EISENHOWER_CANDIDATE_LOCATION-NOTFOUND")
                set(EISENHOWER_QT_LOCATION "${EISENHOWER_CANDIDATE_LOCATION}")
                break()
            endif()
        endforeach()
    endif()
    set("${OUT_VAR}" "${EISENHOWER_QT_LOCATION}" PARENT_SCOPE)
endfunction()

function(eisenhower_chain_deploy_target TARGET_NAME)
    if(NOT TARGET "${TARGET_NAME}")
        return()
    endif()

    get_property(EISENHOWER_PREV_DEPLOY_TARGET GLOBAL PROPERTY EISENHOWER_LAST_DEPLOY_TARGET)
    if(EISENHOWER_PREV_DEPLOY_TARGET AND TARGET "${EISENHOWER_PREV_DEPLOY_TARGET}")
        add_dependencies("${TARGET_NAME}" "${EISENHOWER_PREV_DEPLOY_TARGET}")
    endif()
    set_property(GLOBAL PROPERTY EISENHOWER_LAST_DEPLOY_TARGET "${TARGET_NAME}")
endfunction()

set(EISENHOWER_FORCE_IPO_FOR_PGO OFF)
if(MSVC AND NOT EISENHOWER_PGO_MODE STREQUAL "OFF")
    set(EISENHOWER_FORCE_IPO_FOR_PGO ON)
endif()

set(EISENHOWER_IPO_REQUESTED OFF)
if(EISENHOWER_ENABLE_IPO OR EISENHOWER_FORCE_IPO_FOR_PGO)
    set(EISENHOWER_IPO_REQUESTED ON)
endif()

set(EISENHOWER_IPO_SUPPORTED OFF)
if(EISENHOWER_IPO_REQUESTED)
    check_ipo_supported(RESULT EISENHOWER_IPO_SUPPORTED OUTPUT EISENHOWER_IPO_ERROR LANGUAGES CXX)
    if(NOT EISENHOWER_IPO_SUPPORTED)
        message(WARNING "IPO/LTO is not supported by this toolchain: ${EISENHOWER_IPO_ERROR}")
    endif()
endif()

if(EISENHOWER_FORCE_IPO_FOR_PGO AND NOT EISENHOWER_IPO_SUPPORTED)
    message(FATAL_ERROR "MSVC PGO requires IPO/LTCG support. Disable EISENHOWER_PGO_MODE or use a toolchain with IPO support.")
endif()

if(EISENHOWER_PGO_MODE STREQUAL "USE" AND NOT EXISTS "${EISENHOWER_PGO_DATA_DIR}")
    message(WARNING "EISENHOWER_PGO_MODE=USE was selected, but EISENHOWER_PGO_DATA_DIR does not exist yet: ${EISENHOWER_PGO_DATA_DIR}")
endif()

message(
    STATUS
    "Eisenhower performance profile: IPO=${EISENHOWER_IPO_SUPPORTED}; native=${EISENHOWER_ENABLE_NATIVE_OPTIMIZATIONS}; fast-math=${EISENHOWER_ENABLE_FAST_MATH}; PGO=${EISENHOWER_PGO_MODE}"
)

# Evita picos de memoria limitando linkedicoes simultaneas a 1 no Ninja
set_property(GLOBAL PROPERTY JOB_POOLS link_job_pool=1)
set(CMAKE_JOB_POOL_LINK link_job_pool CACHE STRING "Job pool for linking to avoid memory spikes")

set(EISENHOWER_OPTIMIZED_CONFIG_EXPR "$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>")

function(eisenhower_flag_cache_var PREFIX FLAG OUT_VAR)
    string(MAKE_C_IDENTIFIER "${PREFIX}_${CMAKE_CXX_COMPILER_ID}_${CMAKE_CXX_COMPILER_VERSION}_${FLAG}" FLAG_VAR)
    string(TOUPPER "${FLAG_VAR}" FLAG_VAR)
    set("${OUT_VAR}" "${FLAG_VAR}" PARENT_SCOPE)
endfunction()

function(eisenhower_add_optimized_compile_options TARGET_NAME)
    foreach(FLAG IN LISTS ARGN)
        target_compile_options("${TARGET_NAME}" PRIVATE "$<${EISENHOWER_OPTIMIZED_CONFIG_EXPR}:${FLAG}>")
    endforeach()
endfunction()

function(eisenhower_add_optimized_link_options TARGET_NAME)
    foreach(FLAG IN LISTS ARGN)
        target_link_options("${TARGET_NAME}" PRIVATE "$<${EISENHOWER_OPTIMIZED_CONFIG_EXPR}:${FLAG}>")
    endforeach()
endfunction()

function(eisenhower_add_optimized_compile_option_if_supported TARGET_NAME FLAG)
    eisenhower_flag_cache_var("EISENHOWER_HAS_CXX_FLAG" "${FLAG}" FLAG_VAR)
    check_cxx_compiler_flag("${FLAG}" "${FLAG_VAR}")
    if(${FLAG_VAR})
        eisenhower_add_optimized_compile_options("${TARGET_NAME}" "${FLAG}")
    endif()
endfunction()

function(eisenhower_add_optimized_link_option_if_supported TARGET_NAME FLAG)
    eisenhower_flag_cache_var("EISENHOWER_HAS_LINK_FLAG" "${FLAG}" FLAG_VAR)
    check_linker_flag(CXX "${FLAG}" "${FLAG_VAR}")
    if(${FLAG_VAR})
        eisenhower_add_optimized_link_options("${TARGET_NAME}" "${FLAG}")
    endif()
endfunction()

function(eisenhower_target_pgo_dir TARGET_NAME OUT_VAR)
    file(TO_CMAKE_PATH "${EISENHOWER_PGO_DATA_DIR}/${TARGET_NAME}" EISENHOWER_TARGET_PGO_DIR)
    file(MAKE_DIRECTORY "${EISENHOWER_TARGET_PGO_DIR}")
    set("${OUT_VAR}" "${EISENHOWER_TARGET_PGO_DIR}" PARENT_SCOPE)
endfunction()

function(eisenhower_apply_app_identity TARGET_NAME)
    if(NOT TARGET "${TARGET_NAME}")
        return()
    endif()

    target_compile_definitions("${TARGET_NAME}" PRIVATE "APP_${BUILD_TARGET}")
endfunction()

function(eisenhower_apply_performance_options TARGET_NAME)
    if(NOT TARGET "${TARGET_NAME}")
        return()
    endif()

    eisenhower_apply_app_identity("${TARGET_NAME}")
    eisenhower_protect_autogen_target("${TARGET_NAME}")

    target_compile_definitions(
        "${TARGET_NAME}"
        PRIVATE
            $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
            $<$<CONFIG:Release>:QT_NO_INFO_OUTPUT>
            $<$<CONFIG:RelWithDebInfo>:QT_NO_DEBUG_OUTPUT>
            $<$<CONFIG:RelWithDebInfo>:QT_NO_INFO_OUTPUT>
    )

    if(MSVC)
        set_property(TARGET "${TARGET_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        target_compile_options("${TARGET_NAME}" PRIVATE /Zc:char8_t-)
        if(CMAKE_GENERATOR MATCHES "Visual Studio")
            target_compile_options("${TARGET_NAME}" PRIVATE /MP)
        endif()

        eisenhower_add_optimized_compile_options(
            "${TARGET_NAME}"
            /O2
            /Oi
            /Gy
            /Gw
            /GF
            /Ot
            /Zc:inline
        )

        if(EISENHOWER_ENABLE_FAST_MATH)
            eisenhower_add_optimized_compile_options("${TARGET_NAME}" /fp:fast)
        endif()

        if(EISENHOWER_ENABLE_AGGRESSIVE_INLINING)
            eisenhower_add_optimized_compile_option_if_supported("${TARGET_NAME}" "/Ob3")
        endif()

        if(EISENHOWER_ENABLE_NATIVE_OPTIMIZATIONS AND NOT EISENHOWER_MSVC_ARCH_VALUE STREQUAL "OFF")
            eisenhower_add_optimized_compile_option_if_supported("${TARGET_NAME}" "/arch:${EISENHOWER_MSVC_ARCH_VALUE}")
        endif()

        eisenhower_add_optimized_link_options(
            "${TARGET_NAME}"
            /OPT:REF
            /OPT:ICF
            /INCREMENTAL:NO
            /RELEASE
        )

        if(EISENHOWER_PGO_MODE STREQUAL "GENERATE")
            eisenhower_target_pgo_dir("${TARGET_NAME}" EISENHOWER_TARGET_PGO_DIR)
            set(EISENHOWER_TARGET_PGD "${EISENHOWER_TARGET_PGO_DIR}/${TARGET_NAME}.pgd")
            target_link_options(
                "${TARGET_NAME}"
                PRIVATE
                    "$<${EISENHOWER_OPTIMIZED_CONFIG_EXPR}:/GENPROFILE:PGD=${EISENHOWER_TARGET_PGD}>"
            )
        elseif(EISENHOWER_PGO_MODE STREQUAL "USE")
            eisenhower_target_pgo_dir("${TARGET_NAME}" EISENHOWER_TARGET_PGO_DIR)
            set(EISENHOWER_TARGET_PGD "${EISENHOWER_TARGET_PGO_DIR}/${TARGET_NAME}.pgd")
            target_link_options(
                "${TARGET_NAME}"
                PRIVATE
                    "$<${EISENHOWER_OPTIMIZED_CONFIG_EXPR}:/USEPROFILE:PGD=${EISENHOWER_TARGET_PGD}>"
            )
        endif()
    elseif(MINGW)
        target_compile_options("${TARGET_NAME}" PRIVATE -fno-char8_t)
        set(EISENHOWER_GNU_OPTIMIZATION_FLAG "-O3")
        if(EISENHOWER_ENABLE_FAST_MATH)
            set(EISENHOWER_GNU_OPTIMIZATION_FLAG "-Ofast")
        endif()

        eisenhower_add_optimized_compile_options(
            "${TARGET_NAME}"
            "${EISENHOWER_GNU_OPTIMIZATION_FLAG}"
            -ffunction-sections
            -fdata-sections
            -fomit-frame-pointer
        )

        if(EISENHOWER_ENABLE_AGGRESSIVE_INLINING)
            eisenhower_add_optimized_compile_option_if_supported("${TARGET_NAME}" "-finline-functions")
            eisenhower_add_optimized_compile_option_if_supported("${TARGET_NAME}" "-fipa-pta")
            eisenhower_add_optimized_compile_option_if_supported("${TARGET_NAME}" "-fdevirtualize-at-ltrans")
        endif()

        if(EISENHOWER_ENABLE_NATIVE_OPTIMIZATIONS)
            if(EISENHOWER_GNU_ARCH)
                eisenhower_add_optimized_compile_option_if_supported("${TARGET_NAME}" "-march=${EISENHOWER_GNU_ARCH}")
            endif()
            if(EISENHOWER_GNU_TUNE)
                eisenhower_add_optimized_compile_option_if_supported("${TARGET_NAME}" "-mtune=${EISENHOWER_GNU_TUNE}")
            endif()
        endif()

        eisenhower_add_optimized_link_options(
            "${TARGET_NAME}"
            -Wl,--gc-sections
            -Wl,-O1
        )
        eisenhower_add_optimized_link_option_if_supported("${TARGET_NAME}" "-Wl,--no-keep-memory")

        if(EISENHOWER_STRIP_RELEASE_BINARIES)
            target_link_options("${TARGET_NAME}" PRIVATE $<$<CONFIG:Release>:-Wl,--strip-all>)
        endif()

        if(EISENHOWER_IPO_SUPPORTED)
            eisenhower_add_optimized_link_option_if_supported("${TARGET_NAME}" "-fuse-linker-plugin")
            eisenhower_add_optimized_link_option_if_supported("${TARGET_NAME}" "-Wl,--icf=safe")
        endif()

        if(EISENHOWER_PGO_MODE STREQUAL "GENERATE")
            eisenhower_target_pgo_dir("${TARGET_NAME}" EISENHOWER_TARGET_PGO_DIR)
            eisenhower_add_optimized_compile_options("${TARGET_NAME}" "-fprofile-generate=${EISENHOWER_TARGET_PGO_DIR}")
            eisenhower_add_optimized_link_options("${TARGET_NAME}" "-fprofile-generate=${EISENHOWER_TARGET_PGO_DIR}")
        elseif(EISENHOWER_PGO_MODE STREQUAL "USE")
            eisenhower_target_pgo_dir("${TARGET_NAME}" EISENHOWER_TARGET_PGO_DIR)
            eisenhower_add_optimized_compile_options(
                "${TARGET_NAME}"
                "-fprofile-use=${EISENHOWER_TARGET_PGO_DIR}"
                -fprofile-correction
            )
            eisenhower_add_optimized_link_options("${TARGET_NAME}" "-fprofile-use=${EISENHOWER_TARGET_PGO_DIR}")
        endif()
    endif()

    if(EISENHOWER_IPO_SUPPORTED)
        set_property(TARGET "${TARGET_NAME}" PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET "${TARGET_NAME}" PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    endif()
endfunction()

function(eisenhower_prepare_windows_plugin_deploy_commands OUT_VAR TARGET_NAME WINDEPLOYQT_PATH QTPATHS_PATH)
    set(EISENHOWER_PLUGIN_DEPLOY_COMMANDS)

    if(EISENHOWER_DEPLOY_STANDALONE_PLUGIN_RUNTIME AND EXISTS "${WINDEPLOYQT_PATH}" AND EXISTS "${QTPATHS_PATH}")
        list(
            APPEND
            EISENHOWER_PLUGIN_DEPLOY_COMMANDS
            COMMAND "${WINDEPLOYQT_PATH}"
                "$<TARGET_FILE:${TARGET_NAME}>"
                --dir "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                --libdir "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                --plugindir "${CMAKE_BINARY_DIR}/plugins"
                --translationdir "${CMAKE_BINARY_DIR}/translations"
                ${EISENHOWER_WINDEPLOYQT_COMPILER_RUNTIME_ARGS}
                ${EISENHOWER_WINDEPLOYQT_TRANSLATION_ARGS}
                --qtpaths "${QTPATHS_PATH}"
            COMMAND "${CMAKE_COMMAND}"
                "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/remove_compiler_runtime_installer.cmake"
            COMMAND "${CMAKE_COMMAND}"
                "-DPLUGIN_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/remove_plugin_link_artifacts.cmake"
        )
    else()
        if(EISENHOWER_DEPLOY_STANDALONE_PLUGIN_RUNTIME)
            message(WARNING "windeployqt.exe or qtpaths6.exe was not found. ${TARGET_NAME} will be deployed without a standalone Qt runtime.")
        endif()

        list(
            APPEND
            EISENHOWER_PLUGIN_DEPLOY_COMMANDS
            COMMAND "${CMAKE_COMMAND}"
                "-DPLUGIN_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prune_plugin_runtime_artifacts.cmake"
        )

        if(NOT EISENHOWER_BUILD_HOST_APP)
            list(
                APPEND
                EISENHOWER_PLUGIN_DEPLOY_COMMANDS
                COMMAND "${CMAKE_COMMAND}"
                    "-DDEPLOY_ROOT=${CMAKE_BINARY_DIR}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prune_plugin_runtime_artifacts.cmake"
            )
        endif()
    endif()

    set("${OUT_VAR}" ${EISENHOWER_PLUGIN_DEPLOY_COMMANDS} PARENT_SCOPE)
endfunction()

function(eisenhower_disable_msvc_plugin_import_library TARGET_NAME)
    if(MSVC)
        target_link_options("${TARGET_NAME}" PRIVATE /NOIMPLIB)
    endif()
endfunction()

function(eisenhower_append_optional_qt_dependency_deploy OUT_VAR TARGET_FILE RUNTIME_DIR PLUGIN_DIR TRANSLATION_DIR)
    set(EISENHOWER_DEPLOY_COMMANDS ${${OUT_VAR}})
    list(
        APPEND
        EISENHOWER_DEPLOY_COMMANDS
        COMMAND "${CMAKE_COMMAND}"
            "-DWINDEPLOYQT_EXECUTABLE=${WINDEPLOYQT_EXECUTABLE}"
            "-DQTPATHS_EXECUTABLE=${QTPATHS_EXECUTABLE}"
            "-DTARGET_FILE=${TARGET_FILE}"
            "-DRUNTIME_DIR=${RUNTIME_DIR}"
            "-DPLUGIN_DIR=${PLUGIN_DIR}"
            "-DTRANSLATION_DIR=${TRANSLATION_DIR}"
            "-DQT_TRANSLATIONS=${EISENHOWER_QT_DEPLOY_TRANSLATIONS}"
            "-DNO_COMPILER_RUNTIME=${EISENHOWER_WINDEPLOYQT_NO_COMPILER_RUNTIME}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy_optional_qt_binary.cmake"
    )
    set("${OUT_VAR}" ${EISENHOWER_DEPLOY_COMMANDS} PARENT_SCOPE)
endfunction()

function(eisenhower_append_app_assets_deploy OUT_VAR TARGET_ASSETS_ROOT PRODUCT_ASSETS_DIR LICENSES_DIR PRODUCT_ASSETS_SUBDIR)
    set(EISENHOWER_DEPLOY_COMMANDS ${${OUT_VAR}})
    list(
        APPEND
        EISENHOWER_DEPLOY_COMMANDS
        COMMAND "${CMAKE_COMMAND}"
            "-DTARGET_ASSETS_ROOT=${TARGET_ASSETS_ROOT}"
            "-DPRODUCT_ASSETS_DIR=${PRODUCT_ASSETS_DIR}"
            "-DLICENSES_DIR=${LICENSES_DIR}"
            "-DPRODUCT_ASSETS_SUBDIR=${PRODUCT_ASSETS_SUBDIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_app_assets.cmake"
    )
    set("${OUT_VAR}" ${EISENHOWER_DEPLOY_COMMANDS} PARENT_SCOPE)
endfunction()

if(EISENHOWER_BUILD_MOCKS)
    add_subdirectory(mocks)
endif()

set(EISENHOWER_SHARED_UTIL_SOURCES
    source/utils/ApplicationPathUtils.cpp
    source/utils/CaminhoPersistenteUtils.cpp
    source/utils/IconUtils.cpp
    source/utils/LinkedTitleUnderlineDelegate.cpp
    source/utils/LogManager.cpp
    source/utils/PluginRegistry.cpp
    source/utils/UnifiedTaskStore.cpp
)

set(EISENHOWER_HOST_SOURCES
    main.cpp
    source/eisenhower_app.cpp
    source/app/app_01_eisenhower_matrix_app.hpp
    source/app/app_01_eisenhower_matrix_app.cpp
    source/app/app_02_get_text.hpp
    source/app/app_02_get_text.cpp
    source/app/app_03_initUI.hpp
    source/app/app_03_initUI.cpp
    source/app/app_04_refresh_module_tabs.hpp
    source/app/app_04_refresh_module_tabs.cpp
    source/app/app_05_load_feynman_plugin.hpp
    source/app/app_05_load_feynman_plugin.cpp
    source/app/app_06_send_task_to_feynman.hpp
    source/app/app_06_send_task_to_feynman.cpp
    source/app/app_07_set_task_layout.hpp
    source/app/app_07_set_task_layout.cpp
    source/app/app_08_apply_theme_palette.hpp
    source/app/app_08_apply_theme_palette.cpp
    source/app/app_09_add_task.hpp
    source/app/app_09_add_task.cpp
    source/app/app_10_atualizar_textos.hpp
    source/app/app_10_atualizar_textos.cpp
    source/utils/AppConfig.hpp
    source/utils/ApplicationPathUtils.cpp
    source/utils/BarraTitulosUtils.cpp
    source/utils/CaminhoPersistenteUtils.cpp
    source/utils/IconUtils.cpp
    source/utils/LinkedTitleUnderlineDelegate.cpp
    source/utils/LogManager.cpp
    source/utils/PluginRegistry.cpp
    source/utils/TrialManager.cpp
    source/utils/UnifiedTaskStore.cpp
    source/models/model_01_TaskRecordListModel.cpp
    source/language/tr_01_gerenciadorTraducao.cpp
    source/services/agregador_01_services.cpp
    source/services/incore_15_ProductivityFeatures.cpp
    source/services/RecursosProdutividade/recpro_01_utilitarios_compartilhados.cpp
    source/services/RecursosProdutividade/recpro_02_busca_painel.cpp
    source/services/RecursosProdutividade/recpro_03_recorrencias.cpp
    source/services/RecursosProdutividade/recpro_04_lembretes_locais.cpp
    source/services/RecursosProdutividade/recpro_05_relatorios_fluxos.cpp
    source/services/RecursosProdutividade/recpro_06_revisoes_plugins.cpp
    source/services/RecursosProdutividade/recpro_07_filtros.hpp
    source/services/RecursosProdutividade/recpro_07_filtros.cpp
    source/services/incore_01_initUI.cpp
    source/services/incore_02_add_placeholder.cpp
    source/services/incore_03_criar_menu_configuracoes.cpp
    source/services/incore_04_definir_idioma.cpp
    source/services/incore_05_atualizar_textos.cpp
    source/services/incore_06_atualizar_placeholders.cpp
    source/services/incore_07_MostrarMenuContexto.cpp
    source/services/incore_08_exibir_sobre.cpp
    source/services/incore_09_Arquivo.cpp
    source/services/incore_10_task_list_widget.cpp
    source/services/incore_11_atualizar_itens_tarefas.cpp
    source/services/incore_12_Manual.cpp
    source/services/incore_16_manual_pt-BR.cpp
    source/services/incore_17_manual_en-US.cpp
    source/services/incore_13_prioridade_display.cpp
    source/services/incore_14_EditTaskDialog.cpp
    source/services/Arquivo/arq_01_clear_all_lists.cpp
    source/services/Arquivo/arq_02_add_placeholders_after_clear.cpp
    source/services/Arquivo/arq_03_novo.cpp
    source/services/Arquivo/arq_04_limpar_tudo.cpp
    source/services/Arquivo/arq_05_parse_text_date_time.cpp
    source/services/Arquivo/arq_06_abrir_arquivo.cpp
    source/services/Arquivo/arq_07_salvar_como.cpp
    source/services/MostrarMenuContexto/mmc_01_compute_widget_height.cpp
    source/services/MostrarMenuContexto/mmc_02_apply_widget_min_height.cpp
    source/services/MostrarMenuContexto/mmc_03_effective_locale.cpp
    source/services/MostrarMenuContexto/mmc_04_apply_locale_to_dateedit.cpp
    source/services/MostrarMenuContexto/mmc_05_is_completed_list.cpp
    source/services/MostrarMenuContexto/mmc_06_quadrant_options.cpp
    source/services/MostrarMenuContexto/mmc_07_target_list_for_quadrant.cpp
    source/services/MostrarMenuContexto/mmc_08_find_source_item_for_calendar.cpp
    source/services/MostrarMenuContexto/mmc_09_base_text_from_item.cpp
    source/services/MostrarMenuContexto/mmc_10_build_display_and_tooltip.cpp
    source/services/MostrarMenuContexto/mmc_11_edit_task_dialog.cpp
    source/services/MostrarMenuContexto/mmc_12_move_to_quadrant.cpp
    source/services/MostrarMenuContexto/mmc_13_mostrar_menu_contexto.cpp
    source/ui/agregador_01_ui.cpp
    source/ui/ui_01_SobreDialog.cpp
    source/ui/ui_02_OpcoesSobre.cpp
    source/ui/ui_03_add_task.cpp
    source/ui/ui_04_handle_item_checked.cpp
    source/ui/ui_05_move_item_between_lists.cpp
    source/ui/ui_06_remove_task.cpp
    source/ui/ui_07_save_tasks.cpp
    source/ui/ui_08_load_tasks.cpp
    source/ui/ui_09_Calendar.cpp
    source/ui/ui_10_BadgeCalendarWidget.cpp
    source/ui/ui_11_NativeLineEditFrame.cpp
    source/ui/ui_12_CalendarDialog.cpp
    source/ui/CalendarDialog/agregador_01_CalendarDialog.cpp
    source/ui/CalendarDialog/caldial_01_closeEvent.cpp
    source/ui/CalendarDialog/caldial_02_open_linked_file.cpp
    source/ui/CalendarDialog/caldial_03_collect_tasks.cpp
    source/ui/CalendarDialog/caldial_04_update_task_list.cpp
    source/ui/CalendarDialog/caldial_05_on_calendar_context_menu.cpp
    source/ui/CalendarDialog/caldial_06_prompt_edit_task_for_date.cpp
    source/ui/CalendarDialog/caldial_07_prompt_remove_task_for_date.cpp
    source/ui/CalendarDialog/caldial_08_update_badge_counts.cpp
    source/ui/CalendarDialog/caldial_09_on_language_changed.cpp
    source/ui/CalendarDialog/caldial_10_apply_locale_to_calendar.cpp
    source/ui/CalendarDialog/caldial_11_get_task_dates.cpp
    source/ui/CalendarDialog/caldial_12_apply_highlighted_dates.cpp
    source/ui/ui_13_RotatedTabButton.cpp
    source/ui/ui_14_CalendarPanel.cpp
    source/ui/ui_15_edit_task.cpp
    source/ui/ui_16_YearCalendarWidget.cpp
    source/ui/ui_17_OllamaInstrucoesDialog.cpp
    source/ui/ui_18_ModelDownloadDialog.cpp
)

file(
    GLOB_RECURSE
    EISENHOWER_POMODORO_MODULE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/pomodoro/Pomodoro/Tarefa/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/pomodoro/Pomodoro/PomodoroTimer/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/pomodoro/Pomodoro/GerenciadorTarefas/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/pomodoro/Pomodoro/GerenciadorTempo/*.cpp"
)
list(REMOVE_DUPLICATES EISENHOWER_POMODORO_MODULE_SOURCES)

set(EISENHOWER_POMODORO_PLUGIN_SOURCES
    source/modules/pomodoro/pomodoro_plugin.cpp
    source/modules/pomodoro/Pomodoro/Alarme/sound_01_config_dialog.cpp
    source/modules/pomodoro/Pomodoro/language/tr_01_gerenciadorTraducao.cpp
    source/modules/pomodoro/Pomodoro/pomodoro_support.cpp
    source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.cpp
    source/modules/pomodoro/Pomodoro/tmp_02_PomodoroTimer.cpp
    source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.cpp
    source/modules/pomodoro/Pomodoro/tmp_04_KanbanListWidget.cpp
    source/modules/pomodoro/Pomodoro/tmp_05_GerenciadorTempo.cpp
    source/modules/pomodoro/Pomodoro/tmp_06_TimeWorker.cpp
    ${EISENHOWER_POMODORO_MODULE_SOURCES}
)

set(EISENHOWER_FEYNMAN_PLUGIN_SOURCES
    source/modules/feynman/feynman_app.cpp
    source/modules/feynman/feynman_plugin.cpp
    source/modules/feynman/mod_05_metodo_feynman.cpp
    source/modules/feynman/Feynman/feynman_support.cpp
    source/modules/feynman/Feynman/feynman_metodo_helpers.cpp
    source/modules/feynman/Feynman/fey_01_FeynmanWorker.cpp
    source/modules/feynman/Feynman/fey_02_carregar_conceitos.cpp
    source/modules/feynman/Feynman/fey_03_setup_ui.cpp
    source/modules/feynman/Feynman/fey_04_atualizar_traducoes.cpp
    source/modules/feynman/Feynman/fey_05_novo_conceito.cpp
    source/modules/feynman/Feynman/fey_06_limpar_campos.cpp
    source/modules/feynman/Feynman/fey_07_salvar_conceito_atual.cpp
    source/modules/feynman/Feynman/fey_08_selecionar_conceito.cpp
    source/modules/feynman/Feynman/fey_09_deletar_conceito.cpp
    source/modules/feynman/Feynman/fey_10_atualizar_lista.cpp
    source/modules/feynman/Feynman/fey_11_salvar_conceitos.cpp
    source/modules/feynman/Feynman/fey_12_menu_contexto.cpp
    source/modules/feynman/Feynman/fey_13_receber_conceito_mapa_mental.cpp
    source/modules/feynman/Feynman/fey_14_on_conceitos_loaded.cpp
    source/modules/feynman/Feynman/fey_15_stop_feynman_thread.cpp
    source/modules/feynman/Feynman/fey_16_cleanup.cpp
    source/modules/feynman/Feynman/fey_17_limpar_abas.cpp
    source/modules/feynman/Feynman/fey_18_arquivo_vinculado.cpp
    source/modules/feynman/Feynman/language/tr_01_gerenciadorTraducao.cpp
    source/modules/feynman/Feynman/utils/EventBus.cpp
    source/modules/feynman/Feynman/utils/GerenciadorBotoesUI.cpp
    source/modules/feynman/Feynman/utils/MessageBox.cpp
)

file(
    GLOB_RECURSE
    EISENHOWER_MAPAMENTAL_PLUGIN_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/mapamental/*.cpp"
)
list(
    APPEND
    EISENHOWER_MAPAMENTAL_PLUGIN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/mapamental/metodoMapaMental/language/tr_01_gerenciadorTraducao.cpp"
)
list(
    FILTER
    EISENHOWER_MAPAMENTAL_PLUGIN_SOURCES
    EXCLUDE
    REGEX ".*/language/.*CompileTranslations.*\\.cpp$"
)
list(REMOVE_DUPLICATES EISENHOWER_MAPAMENTAL_PLUGIN_SOURCES)

set(EISENHOWER_MODELO_PLUGIN_SOURCES
    source/modules/modelo/modelo_plugin.cpp
    source/modules/modelo/metodoModelo/qwen/ia_01_llama_local.cpp
    source/modules/modelo/metodoModelo/ollama/ia_02_ollama_local.cpp
)

file(
    GLOB_RECURSE
    EISENHOWER_AUDIOLIVRO_PLUGIN_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/audiolivro/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/audiolivro/*.hpp"
)
list(
    FILTER
    EISENHOWER_AUDIOLIVRO_PLUGIN_SOURCES
    EXCLUDE
    REGEX ".*/language/.*CompileTranslations.*\\.(cpp|hpp)$"
)
list(REMOVE_DUPLICATES EISENHOWER_AUDIOLIVRO_PLUGIN_SOURCES)

file(
    GLOB_RECURSE
    EISENHOWER_AGENDA_MODULE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/agenda/AgendaAvaliacoes/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/agenda/AgendaAvaliacoes/*.hpp"
)
set(EISENHOWER_AGENDA_PLUGIN_SOURCES
    source/modules/agenda/agenda_plugin.cpp
    ${EISENHOWER_AGENDA_MODULE_SOURCES}
)
list(REMOVE_DUPLICATES EISENHOWER_AGENDA_PLUGIN_SOURCES)

file(
    GLOB_RECURSE
    EISENHOWER_GANTT_MODULE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/gantt/Gantt/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/modules/gantt/Gantt/*.hpp"
)
set(EISENHOWER_GANTT_PLUGIN_SOURCES
    source/modules/gantt/gantt_plugin.cpp
    ${EISENHOWER_GANTT_MODULE_SOURCES}
)
list(REMOVE_DUPLICATES EISENHOWER_GANTT_PLUGIN_SOURCES)

if(NOT EISENHOWER_MODULAR_PLUGINS)
    list(APPEND EISENHOWER_HOST_SOURCES
        ${EISENHOWER_POMODORO_PLUGIN_SOURCES}
        ${EISENHOWER_FEYNMAN_PLUGIN_SOURCES}
        ${EISENHOWER_MAPAMENTAL_PLUGIN_SOURCES}
        ${EISENHOWER_MODELO_PLUGIN_SOURCES}
        ${EISENHOWER_AUDIOLIVRO_PLUGIN_SOURCES}
        ${EISENHOWER_AGENDA_PLUGIN_SOURCES}
        ${EISENHOWER_GANTT_PLUGIN_SOURCES}
    )
    list(REMOVE_DUPLICATES EISENHOWER_HOST_SOURCES)
endif()

set(EISENHOWER_HOST_LIB_SOURCES ${EISENHOWER_HOST_SOURCES})
if(EISENHOWER_MODULAR_PLUGINS)
    list(APPEND EISENHOWER_HOST_LIB_SOURCES
        ${EISENHOWER_POMODORO_PLUGIN_SOURCES}
        ${EISENHOWER_FEYNMAN_PLUGIN_SOURCES}
        ${EISENHOWER_MAPAMENTAL_PLUGIN_SOURCES}
        ${EISENHOWER_MODELO_PLUGIN_SOURCES}
        ${EISENHOWER_AUDIOLIVRO_PLUGIN_SOURCES}
        ${EISENHOWER_AGENDA_PLUGIN_SOURCES}
        ${EISENHOWER_GANTT_PLUGIN_SOURCES}
    )
    list(REMOVE_DUPLICATES EISENHOWER_HOST_LIB_SOURCES)
endif()
list(REMOVE_ITEM EISENHOWER_HOST_LIB_SOURCES main.cpp "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

if(EISENHOWER_BUILD_HOST_APP)
    add_executable(EisenhowerOrganizer WIN32 ${EISENHOWER_HOST_SOURCES})
    set_target_properties(EisenhowerOrganizer PROPERTIES OUTPUT_NAME "${APP_EXECUTABLE_NAME}")

    target_include_directories(EisenhowerOrganizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    if(EISENHOWER_MODULAR_PLUGINS)
        target_compile_definitions(EisenhowerOrganizer PRIVATE EISENHOWER_MODULAR_PLUGINS=1)
        target_link_libraries(EisenhowerOrganizer
            PRIVATE
                Qt6::Core
                Qt6::Gui
                Qt6::Widgets
                Qt6::Network
                Qt6::Sql
                Qt6::CorePrivate
                Advapi32
                User32
        )
    else()
        target_link_libraries(EisenhowerOrganizer
            PRIVATE
                Qt6::Core
                Qt6::Gui
                Qt6::Widgets
                Qt6::Multimedia
                Qt6::Network
                Qt6::OpenGLWidgets
                Qt6::Pdf
                Qt6::PdfWidgets
                Qt6::Sql
                Qt6::WebSockets
                Qt6::CorePrivate
                Advapi32
                User32
        )
    endif()

    eisenhower_apply_performance_options(EisenhowerOrganizer)

    if(WIN32)
        set(EISENHOWER_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/assets")
        set(EISENHOWER_APP_ASSETS_DIR "${EISENHOWER_ASSETS_DIR}/${APP_ASSETS_SUBDIR}")
        set(EISENHOWER_LICENSES_DIR "${EISENHOWER_ASSETS_DIR}/LICENSES")
        set(EISENHOWER_APP_ICON "${EISENHOWER_APP_ASSETS_DIR}/icones/${APP_WINDOWS_ICON_NAME}")
        set(EISENHOWER_TRANSLATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations")
        set(EISENHOWER_BUILD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/build")
        set(EISENHOWER_FEYNMAN_PLUGIN_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_feynman_${APP_PLUGIN_BUILD_SUFFIX}/plugins/feynman")
        set(EISENHOWER_POMODORO_PLUGIN_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_pomodoro_${APP_PLUGIN_BUILD_SUFFIX}/plugins/pomodoro")
        set(EISENHOWER_MAPAMENTAL_PLUGIN_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_mapamental_${APP_PLUGIN_BUILD_SUFFIX}/plugins/mapamental")
        set(EISENHOWER_MODELO_PLUGIN_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_modelo_${APP_PLUGIN_BUILD_SUFFIX}/plugins/modelo")
        set(EISENHOWER_AUDIOLIVRO_PLUGIN_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_audiolivro_${APP_PLUGIN_BUILD_SUFFIX}/plugins/audiolivro")
        set(EISENHOWER_AGENDA_PLUGIN_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_agenda_${APP_PLUGIN_BUILD_SUFFIX}/plugins/agenda")
        set(EISENHOWER_GANTT_PLUGIN_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_gantt_${APP_PLUGIN_BUILD_SUFFIX}/plugins/gantt")
        set(EISENHOWER_MAPAMENTAL_SQLDRIVERS_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_mapamental_${APP_PLUGIN_BUILD_SUFFIX}/plugins/sqldrivers")
        set(EISENHOWER_MULTIMEDIA_PLUGIN_EXPORT_SOURCE "${EISENHOWER_BUILD_ROOT}/build_pomodoro_${APP_PLUGIN_BUILD_SUFFIX}/plugins/multimedia")
        if(NOT EXISTS "${EISENHOWER_APP_ASSETS_DIR}")
            message(FATAL_ERROR "Application assets directory was not found at ${EISENHOWER_APP_ASSETS_DIR}.")
        endif()
        if(NOT EXISTS "${EISENHOWER_LICENSES_DIR}")
            message(FATAL_ERROR "Application licenses directory was not found at ${EISENHOWER_LICENSES_DIR}.")
        endif()
        if(EXISTS "${EISENHOWER_APP_ICON}")
            file(TO_NATIVE_PATH "${EISENHOWER_APP_ICON}" EISENHOWER_APP_ICON_NATIVE)
            string(REPLACE "\\" "\\\\" EISENHOWER_APP_ICON_RC_PATH "${EISENHOWER_APP_ICON_NATIVE}")
            configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/source/windows_app_icon.rc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/EisenhowerOrganizer_app_icon.rc"
                @ONLY
            )
            target_sources(
                EisenhowerOrganizer
                PRIVATE
                    "${CMAKE_CURRENT_BINARY_DIR}/EisenhowerOrganizer_app_icon.rc"
            )
        else()
            message(WARNING "Application icon was not found at ${EISENHOWER_APP_ICON}. The Windows executable will be built without the requested icon.")
        endif()

        file(TO_CMAKE_PATH "${EISENHOWER_EXPORT_DIR}" EISENHOWER_EXPORT_DIR_NORMALIZED)
        get_filename_component(QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
        set(WINDEPLOYQT_EXECUTABLE "${QT6_PREFIX_DIR}/bin/windeployqt.exe")
        set(QTPATHS_EXECUTABLE "${QT6_PREFIX_DIR}/bin/qtpaths6.exe")

        if(EXISTS "${WINDEPLOYQT_EXECUTABLE}" AND EXISTS "${QTPATHS_EXECUTABLE}")
            set(EISENHOWER_BUILD_TREE_PLUGIN_QT_DEPLOY_COMMANDS)
            set(EISENHOWER_EXPORT_PLUGIN_QT_DEPLOY_COMMANDS)

            set(EISENHOWER_BUILD_TREE_OPTIONAL_ROOT_RUNTIME_PRUNE_COMMANDS)
            set(EISENHOWER_EXPORT_OPTIONAL_ROOT_RUNTIME_PRUNE_COMMANDS)
            set(EISENHOWER_PRUNE_ROOT_MSVC_RUNTIME_DLLS OFF)
            set(EISENHOWER_PRUNE_ROOT_MINGW_RUNTIME_DLLS OFF)
            if(WIN32 AND MSVC)
                set(EISENHOWER_PRUNE_ROOT_MINGW_RUNTIME_DLLS ON)
            elseif(WIN32 AND NOT MSVC)
                set(EISENHOWER_PRUNE_ROOT_MSVC_RUNTIME_DLLS ON)
            endif()
            set(EISENHOWER_BUILD_TREE_ASSET_COPY_COMMANDS)
            eisenhower_append_app_assets_deploy(
                EISENHOWER_BUILD_TREE_ASSET_COPY_COMMANDS
                "$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                "${EISENHOWER_APP_ASSETS_DIR}"
                "${EISENHOWER_LICENSES_DIR}"
                "${APP_ASSETS_SUBDIR}"
            )
            set(EISENHOWER_EXPORT_ASSET_COPY_COMMANDS)
            eisenhower_append_app_assets_deploy(
                EISENHOWER_EXPORT_ASSET_COPY_COMMANDS
                "${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                "${EISENHOWER_APP_ASSETS_DIR}"
                "${EISENHOWER_LICENSES_DIR}"
                "${APP_ASSETS_SUBDIR}"
            )
            if(EISENHOWER_PRUNE_OPTIONAL_ROOT_RUNTIME_DLLS)
                list(
                    APPEND
                    EISENHOWER_BUILD_TREE_OPTIONAL_ROOT_RUNTIME_PRUNE_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                        "-DPRUNE_MSVC_RUNTIME=${EISENHOWER_PRUNE_ROOT_MSVC_RUNTIME_DLLS}"
                        "-DPRUNE_MINGW_RUNTIME=${EISENHOWER_PRUNE_ROOT_MINGW_RUNTIME_DLLS}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prune_optional_root_runtime_dlls.cmake"
                )
                list(
                    APPEND
                    EISENHOWER_EXPORT_OPTIONAL_ROOT_RUNTIME_PRUNE_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DDEPLOY_ROOT=${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                        "-DPRUNE_MSVC_RUNTIME=${EISENHOWER_PRUNE_ROOT_MSVC_RUNTIME_DLLS}"
                        "-DPRUNE_MINGW_RUNTIME=${EISENHOWER_PRUNE_ROOT_MINGW_RUNTIME_DLLS}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prune_optional_root_runtime_dlls.cmake"
                )
            endif()

            set(EISENHOWER_BUILD_TREE_MSVC_RUNTIME_COMMANDS)
            set(EISENHOWER_EXPORT_MSVC_RUNTIME_COMMANDS)
            if(MSVC)
                list(
                    APPEND
                    EISENHOWER_BUILD_TREE_MSVC_RUNTIME_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                        "-DMSVC_REDIST_ROOT=${EISENHOWER_MSVC_REDIST_ROOT}"
                        "-DMSVC_RUNTIME_ARCH=${EISENHOWER_MSVC_RUNTIME_ARCH}"
                        "-DCONFIG=$<CONFIG>"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_msvc_runtime.cmake"
                )
                list(
                    APPEND
                    EISENHOWER_EXPORT_MSVC_RUNTIME_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DDEPLOY_ROOT=${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                        "-DMSVC_REDIST_ROOT=${EISENHOWER_MSVC_REDIST_ROOT}"
                        "-DMSVC_RUNTIME_ARCH=${EISENHOWER_MSVC_RUNTIME_ARCH}"
                        "-DCONFIG=$<CONFIG>"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_msvc_runtime.cmake"
                )
            elseif(MINGW)
                list(
                    APPEND
                    EISENHOWER_BUILD_TREE_MSVC_RUNTIME_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                        "-DMINGW_BIN_DIR=${EISENHOWER_MINGW_BIN_DIR}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_mingw_runtime.cmake"
                )
                list(
                    APPEND
                    EISENHOWER_EXPORT_MSVC_RUNTIME_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DDEPLOY_ROOT=${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                        "-DMINGW_BIN_DIR=${EISENHOWER_MINGW_BIN_DIR}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_mingw_runtime.cmake"
                )
            endif()

            set(EISENHOWER_BUILD_TREE_MODELO_MSVC_RUNTIME_COMMANDS)
            set(EISENHOWER_EXPORT_MODELO_MSVC_RUNTIME_COMMANDS)
            if(NOT EISENHOWER_MSVC_REDIST_ROOT STREQUAL "")
                list(
                    APPEND
                    EISENHOWER_BUILD_TREE_MODELO_MSVC_RUNTIME_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DMODELO_RUNTIME_DIR=$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                        "-DMSVC_REDIST_ROOT=${EISENHOWER_MSVC_REDIST_ROOT}"
                        "-DMSVC_RUNTIME_ARCH=${EISENHOWER_MSVC_RUNTIME_ARCH}"
                        "-DCONFIG=Release"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modelo_msvc_runtime.cmake"
                )
                list(
                    APPEND
                    EISENHOWER_EXPORT_MODELO_MSVC_RUNTIME_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DMODELO_RUNTIME_DIR=${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                        "-DMSVC_REDIST_ROOT=${EISENHOWER_MSVC_REDIST_ROOT}"
                        "-DMSVC_RUNTIME_ARCH=${EISENHOWER_MSVC_RUNTIME_ARCH}"
                        "-DCONFIG=Release"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modelo_msvc_runtime.cmake"
                )
            endif()

            set(EISENHOWER_BUILD_TREE_EXTRA_COMMANDS)
            set(EISENHOWER_EXPORT_EXTRA_COMMANDS)
            if(EISENHOWER_MODULAR_PLUGINS)
                set(EISENHOWER_MODULAR_BUILD_TREE_DLLS
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins/feynman/FeynmanPlugin.dll"
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins/pomodoro/PomodoroPlugin.dll"
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins/mapamental/MapaMentalPlugin.dll"
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins/modelo/ModeloPlugin.dll"
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins/audiolivro/AudiolivroPlugin.dll"
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins/agenda/AgendaPlugin.dll"
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins/gantt/GanttPlugin.dll"
                )
                set(EISENHOWER_MODULAR_EXPORT_DLLS
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins/feynman/FeynmanPlugin.dll"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins/pomodoro/PomodoroPlugin.dll"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins/mapamental/MapaMentalPlugin.dll"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins/modelo/ModeloPlugin.dll"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins/audiolivro/AudiolivroPlugin.dll"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins/agenda/AgendaPlugin.dll"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins/gantt/GanttPlugin.dll"
                )
                list(
                    APPEND
                    EISENHOWER_BUILD_TREE_EXTRA_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DBUILD_ROOT=${EISENHOWER_BUILD_ROOT}"
                        "-DTOOLCHAIN_SUFFIX=${APP_PLUGIN_BUILD_SUFFIX}"
                        "-DDEST_ROOT=$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                        "-DEMBED_GGUF_MODEL=${EISENHOWER_EMBED_GGUF_MODEL}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modular_plugins.cmake"
                )
                list(
                    APPEND
                    EISENHOWER_EXPORT_EXTRA_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        "-DBUILD_ROOT=${EISENHOWER_BUILD_ROOT}"
                        "-DTOOLCHAIN_SUFFIX=${APP_PLUGIN_BUILD_SUFFIX}"
                        "-DDEST_ROOT=${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                        "-DEMBED_GGUF_MODEL=${EISENHOWER_EMBED_GGUF_MODEL}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modular_plugins.cmake"
                )
                foreach(EISENHOWER_MODULAR_PLUGIN_DLL IN LISTS EISENHOWER_MODULAR_BUILD_TREE_DLLS)
                    eisenhower_append_optional_qt_dependency_deploy(
                        EISENHOWER_BUILD_TREE_EXTRA_COMMANDS
                        "${EISENHOWER_MODULAR_PLUGIN_DLL}"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/translations"
                    )
                endforeach()
                foreach(EISENHOWER_MODULAR_PLUGIN_DLL IN LISTS EISENHOWER_MODULAR_EXPORT_DLLS)
                    eisenhower_append_optional_qt_dependency_deploy(
                        EISENHOWER_EXPORT_EXTRA_COMMANDS
                        "${EISENHOWER_MODULAR_PLUGIN_DLL}"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/translations"
                    )
                endforeach()
                if(NOT EISENHOWER_MSVC_REDIST_ROOT STREQUAL "")
                    list(
                        APPEND
                        EISENHOWER_BUILD_TREE_EXTRA_COMMANDS
                        COMMAND "${CMAKE_COMMAND}"
                            "-DMODELO_RUNTIME_DIR=$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins/modelo"
                            "-DMSVC_REDIST_ROOT=${EISENHOWER_MSVC_REDIST_ROOT}"
                            "-DMSVC_RUNTIME_ARCH=${EISENHOWER_MSVC_RUNTIME_ARCH}"
                            "-DCONFIG=Release"
                            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modelo_msvc_runtime.cmake"
                    )
                    list(
                        APPEND
                        EISENHOWER_EXPORT_EXTRA_COMMANDS
                        COMMAND "${CMAKE_COMMAND}"
                            "-DMODELO_RUNTIME_DIR=${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins/modelo"
                            "-DMSVC_REDIST_ROOT=${EISENHOWER_MSVC_REDIST_ROOT}"
                            "-DMSVC_RUNTIME_ARCH=${EISENHOWER_MSVC_RUNTIME_ARCH}"
                            "-DCONFIG=Release"
                            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modelo_msvc_runtime.cmake"
                    )
                endif()
                list(
                    APPEND
                    EISENHOWER_BUILD_TREE_EXTRA_COMMANDS
                    COMMAND "${CMAKE_COMMAND}" -E rm -rf
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/language"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/Alarme"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/sound"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/llama"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/models"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/qml"
                )
                list(
                    APPEND
                    EISENHOWER_EXPORT_EXTRA_COMMANDS
                    COMMAND "${CMAKE_COMMAND}" -E rm -rf
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/language"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/Alarme"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/sound"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/llama"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/models"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/qml"
                )
            else()
                list(
                    APPEND
                    EISENHOWER_BUILD_TREE_EXTRA_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/pomodoro/Pomodoro/Alarme/sound
                        -DDEST_DIR=$<TARGET_FILE_DIR:EisenhowerOrganizer>/sound
                        -DCLEAN_DESTINATION=ON
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_optional_directory.cmake"
                    COMMAND "${CMAKE_COMMAND}" -E rm -rf
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/language"
                        "$<TARGET_FILE_DIR:EisenhowerOrganizer>/Alarme"
                    COMMAND "${CMAKE_COMMAND}"
                        "-DMODELS_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/models"
                        "-DCPU_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/llama-b8987-bin-win-cpu-x64"
                        "-DVULKAN_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/llama-b8987-bin-win-vulkan-x64"
                        "-DDEST_DIR=$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                        "-DEMBED_GGUF_MODEL=${EISENHOWER_EMBED_GGUF_MODEL}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modelo_runtime.cmake"
                    ${EISENHOWER_BUILD_TREE_MODELO_MSVC_RUNTIME_COMMANDS}
                )
                list(
                    APPEND
                    EISENHOWER_EXPORT_EXTRA_COMMANDS
                    COMMAND "${CMAKE_COMMAND}"
                        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/pomodoro/Pomodoro/Alarme/sound
                        -DDEST_DIR=${EISENHOWER_EXPORT_DIR_NORMALIZED}/sound
                        -DCLEAN_DESTINATION=ON
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_optional_directory.cmake"
                    COMMAND "${CMAKE_COMMAND}" -E rm -rf
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/language"
                        "${EISENHOWER_EXPORT_DIR_NORMALIZED}/Alarme"
                    COMMAND "${CMAKE_COMMAND}"
                        "-DMODELS_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/models"
                        "-DCPU_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/llama-b8987-bin-win-cpu-x64"
                        "-DVULKAN_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/llama-b8987-bin-win-vulkan-x64"
                        "-DDEST_DIR=${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                        "-DEMBED_GGUF_MODEL=${EISENHOWER_EMBED_GGUF_MODEL}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modelo_runtime.cmake"
                    ${EISENHOWER_EXPORT_MODELO_MSVC_RUNTIME_COMMANDS}
                )
            endif()

            add_custom_target(
                EisenhowerOrganizerDeploy
                ALL
                DEPENDS EisenhowerOrganizer
                COMMAND "${CMAKE_COMMAND}" -E rm -rf
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/translations"
                    "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins"
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    "$<TARGET_FILE:EisenhowerOrganizer>"
                    --dir "$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                    --libdir "$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                    --plugindir "$<TARGET_FILE_DIR:EisenhowerOrganizer>/plugins"
                    --qml-deploy-dir "$<TARGET_FILE_DIR:EisenhowerOrganizer>/qml"
                    --translationdir "$<TARGET_FILE_DIR:EisenhowerOrganizer>/translations"
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    ${EISENHOWER_WINDEPLOYQT_COMPILER_RUNTIME_ARGS}
                    ${EISENHOWER_WINDEPLOYQT_TRANSLATION_ARGS}
                    --qtpaths "${QTPATHS_EXECUTABLE}"
                COMMAND "${CMAKE_COMMAND}"
                    "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/remove_compiler_runtime_installer.cmake"
                ${EISENHOWER_BUILD_TREE_ASSET_COPY_COMMANDS}
                COMMAND "${CMAKE_COMMAND}"
                    "-DSOURCE_DIR=${EISENHOWER_TRANSLATIONS_DIR}"
                    "-DDEST_DIR=$<TARGET_FILE_DIR:EisenhowerOrganizer>/translations"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_qm_translations.cmake"
                ${EISENHOWER_BUILD_TREE_EXTRA_COMMANDS}
                ${EISENHOWER_BUILD_TREE_MSVC_RUNTIME_COMMANDS}
                ${EISENHOWER_BUILD_TREE_OPTIONAL_ROOT_RUNTIME_PRUNE_COMMANDS}
                COMMAND "${CMAKE_COMMAND}"
                    "-DDEPLOY_ROOT=$<TARGET_FILE_DIR:EisenhowerOrganizer>"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prune_optional_qt_sqldrivers.cmake"
                COMMAND "${CMAKE_COMMAND}" -E rm -rf
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/translations"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/qml"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/assets"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/sound"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/llama"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/models"
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "$<TARGET_FILE:EisenhowerOrganizer>"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/"
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    "${EISENHOWER_EXPORT_DIR_NORMALIZED}/$<TARGET_FILE_NAME:EisenhowerOrganizer>"
                    --dir "${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                    --libdir "${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                    --plugindir "${EISENHOWER_EXPORT_DIR_NORMALIZED}/plugins"
                    --qml-deploy-dir "${EISENHOWER_EXPORT_DIR_NORMALIZED}/qml"
                    --translationdir "${EISENHOWER_EXPORT_DIR_NORMALIZED}/translations"
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    ${EISENHOWER_WINDEPLOYQT_COMPILER_RUNTIME_ARGS}
                    ${EISENHOWER_WINDEPLOYQT_TRANSLATION_ARGS}
                    --qtpaths "${QTPATHS_EXECUTABLE}"
                COMMAND "${CMAKE_COMMAND}"
                    "-DDEPLOY_ROOT=${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/remove_compiler_runtime_installer.cmake"
                ${EISENHOWER_EXPORT_ASSET_COPY_COMMANDS}
                COMMAND "${CMAKE_COMMAND}"
                    "-DSOURCE_DIR=${EISENHOWER_TRANSLATIONS_DIR}"
                    "-DDEST_DIR=${EISENHOWER_EXPORT_DIR_NORMALIZED}/translations"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_qm_translations.cmake"
                ${EISENHOWER_EXPORT_EXTRA_COMMANDS}
                ${EISENHOWER_EXPORT_MSVC_RUNTIME_COMMANDS}
                ${EISENHOWER_EXPORT_OPTIONAL_ROOT_RUNTIME_PRUNE_COMMANDS}
                COMMAND "${CMAKE_COMMAND}"
                    "-DDEPLOY_ROOT=${EISENHOWER_EXPORT_DIR_NORMALIZED}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prune_optional_qt_sqldrivers.cmake"
                COMMENT "Deploying EisenhowerOrganizer runtime to build tree and export directory"
                VERBATIM
            )
            eisenhower_chain_deploy_target(EisenhowerOrganizerDeploy)
        else()
            message(WARNING "windeployqt.exe or qtpaths6.exe was not found. The build tree executable may miss Qt runtime DLLs.")
        endif()
    endif()
endif()

if(EISENHOWER_BUILD_FEYNMAN_PLUGIN)
    add_library(FeynmanPlugin MODULE
        ${EISENHOWER_FEYNMAN_PLUGIN_SOURCES}
        ${EISENHOWER_SHARED_UTIL_SOURCES}
        source/ui/ui_13_RotatedTabButton.cpp
    )

    target_include_directories(FeynmanPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(FeynmanPlugin PRIVATE QT_PLUGIN EISENHOWER_BUILD_FEYNMAN_PLUGIN_DLL=1)

    target_link_libraries(FeynmanPlugin
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
            Advapi32
            User32
    )

    eisenhower_apply_performance_options(FeynmanPlugin)
    eisenhower_disable_msvc_plugin_import_library(FeynmanPlugin)

    set(EISENHOWER_FEYNMAN_PLUGIN_DIR "${CMAKE_BINARY_DIR}/plugins/feynman")
    file(MAKE_DIRECTORY "${EISENHOWER_FEYNMAN_PLUGIN_DIR}")
    set_target_properties(
        FeynmanPlugin
        PROPERTIES
            OUTPUT_NAME "FeynmanPlugin"
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${EISENHOWER_FEYNMAN_PLUGIN_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EISENHOWER_FEYNMAN_PLUGIN_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${EISENHOWER_FEYNMAN_PLUGIN_DIR}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${CONFIG}" CONFIG_UPPER)
            set_target_properties(
                FeynmanPlugin
                PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_FEYNMAN_PLUGIN_DIR}"
                    RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_FEYNMAN_PLUGIN_DIR}"
                    ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_FEYNMAN_PLUGIN_DIR}"
            )
        endforeach()
    endif()

    if(WIN32)
        get_filename_component(QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
        set(WINDEPLOYQT_EXECUTABLE "${QT6_PREFIX_DIR}/bin/windeployqt.exe")
        set(QTPATHS_EXECUTABLE "${QT6_PREFIX_DIR}/bin/qtpaths6.exe")

        eisenhower_prepare_windows_plugin_deploy_commands(
            EISENHOWER_FEYNMAN_PLUGIN_DEPLOY_COMMANDS
            FeynmanPlugin
            "${WINDEPLOYQT_EXECUTABLE}"
            "${QTPATHS_EXECUTABLE}"
        )
        add_custom_target(
            FeynmanPluginDeploy
            ALL
            DEPENDS FeynmanPlugin
            ${EISENHOWER_FEYNMAN_PLUGIN_DEPLOY_COMMANDS}
            COMMAND "${CMAKE_COMMAND}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations"
                "-DDEST_DIR=$<TARGET_FILE_DIR:FeynmanPlugin>/translations"
                "-DFILE_PATTERN=feynman_*.qm"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_qm_translations.cmake"
            COMMENT "Deploying FeynmanPlugin package without duplicated app runtime"
            VERBATIM
        )
        eisenhower_chain_deploy_target(FeynmanPluginDeploy)
    endif()
endif()

if(EISENHOWER_BUILD_MAPAMENTAL_PLUGIN)
    add_library(MapaMentalPlugin MODULE
        ${EISENHOWER_MAPAMENTAL_PLUGIN_SOURCES}
        ${EISENHOWER_SHARED_UTIL_SOURCES}
    )

    target_include_directories(MapaMentalPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(MapaMentalPlugin PRIVATE QT_PLUGIN EISENHOWER_BUILD_MAPAMENTAL_PLUGIN_DLL=1)

    target_link_libraries(MapaMentalPlugin
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::OpenGLWidgets
            Qt6::Pdf
            Qt6::Sql
            Qt6::Widgets
            Qt6::CorePrivate
            Advapi32
            User32
    )

    eisenhower_apply_performance_options(MapaMentalPlugin)
    eisenhower_disable_msvc_plugin_import_library(MapaMentalPlugin)

    set(EISENHOWER_MAPAMENTAL_PLUGIN_DIR "${CMAKE_BINARY_DIR}/plugins/mapamental")
    file(MAKE_DIRECTORY "${EISENHOWER_MAPAMENTAL_PLUGIN_DIR}")
    set_target_properties(
        MapaMentalPlugin
        PROPERTIES
            OUTPUT_NAME "MapaMentalPlugin"
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${EISENHOWER_MAPAMENTAL_PLUGIN_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EISENHOWER_MAPAMENTAL_PLUGIN_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${EISENHOWER_MAPAMENTAL_PLUGIN_DIR}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${CONFIG}" CONFIG_UPPER)
            set_target_properties(
                MapaMentalPlugin
                PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_MAPAMENTAL_PLUGIN_DIR}"
                    RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_MAPAMENTAL_PLUGIN_DIR}"
                    ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_MAPAMENTAL_PLUGIN_DIR}"
            )
        endforeach()
    endif()

    if(WIN32)
        get_filename_component(QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
        set(WINDEPLOYQT_EXECUTABLE "${QT6_PREFIX_DIR}/bin/windeployqt.exe")
        set(QTPATHS_EXECUTABLE "${QT6_PREFIX_DIR}/bin/qtpaths6.exe")

        eisenhower_prepare_windows_plugin_deploy_commands(
            EISENHOWER_MAPAMENTAL_PLUGIN_DEPLOY_COMMANDS
            MapaMentalPlugin
            "${WINDEPLOYQT_EXECUTABLE}"
            "${QTPATHS_EXECUTABLE}"
        )
        add_custom_target(
            MapaMentalPluginDeploy
            ALL
            DEPENDS MapaMentalPlugin
            ${EISENHOWER_MAPAMENTAL_PLUGIN_DEPLOY_COMMANDS}
            COMMAND "${CMAKE_COMMAND}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations"
                "-DDEST_DIR=$<TARGET_FILE_DIR:MapaMentalPlugin>/translations"
                "-DFILE_PATTERN=mapamental_*.qm"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_qm_translations.cmake"
            COMMAND "${CMAKE_COMMAND}" -E remove_directory
                "$<TARGET_FILE_DIR:MapaMentalPlugin>/models"
            COMMAND "${CMAKE_COMMAND}" -E remove_directory
                "$<TARGET_FILE_DIR:MapaMentalPlugin>/llama"
            COMMENT "Deploying compact MapaMentalPlugin package with heuristic text processing"
            VERBATIM
        )
        eisenhower_chain_deploy_target(MapaMentalPluginDeploy)
    endif()
endif()

if(EISENHOWER_BUILD_MODELO_PLUGIN)
    add_library(ModeloPlugin MODULE
        ${EISENHOWER_MODELO_PLUGIN_SOURCES}
        source/utils/ApplicationPathUtils.cpp
        source/utils/CaminhoPersistenteUtils.cpp
        source/utils/LogManager.cpp
    )

    target_include_directories(ModeloPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(ModeloPlugin PRIVATE QT_PLUGIN EISENHOWER_BUILD_MODELO_PLUGIN_DLL=1)
    target_link_libraries(ModeloPlugin PRIVATE Qt6::Core Qt6::Network Advapi32 User32)
    eisenhower_apply_performance_options(ModeloPlugin)
    eisenhower_disable_msvc_plugin_import_library(ModeloPlugin)

    set(EISENHOWER_MODELO_PLUGIN_DIR "${CMAKE_BINARY_DIR}/plugins/modelo")
    file(MAKE_DIRECTORY "${EISENHOWER_MODELO_PLUGIN_DIR}")
    set_target_properties(
        ModeloPlugin
        PROPERTIES
            OUTPUT_NAME "ModeloPlugin"
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${EISENHOWER_MODELO_PLUGIN_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EISENHOWER_MODELO_PLUGIN_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${EISENHOWER_MODELO_PLUGIN_DIR}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${CONFIG}" CONFIG_UPPER)
            set_target_properties(
                ModeloPlugin
                PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_MODELO_PLUGIN_DIR}"
                    RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_MODELO_PLUGIN_DIR}"
                    ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_MODELO_PLUGIN_DIR}"
            )
        endforeach()
    endif()

    if(WIN32)
        get_filename_component(QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
        set(WINDEPLOYQT_EXECUTABLE "${QT6_PREFIX_DIR}/bin/windeployqt.exe")
        set(QTPATHS_EXECUTABLE "${QT6_PREFIX_DIR}/bin/qtpaths6.exe")
        eisenhower_prepare_windows_plugin_deploy_commands(
            EISENHOWER_MODELO_PLUGIN_DEPLOY_COMMANDS
            ModeloPlugin
            "${WINDEPLOYQT_EXECUTABLE}"
            "${QTPATHS_EXECUTABLE}"
        )
        set(EISENHOWER_MODELO_PLUGIN_MSVC_RUNTIME_COMMANDS)
        if(NOT EISENHOWER_MSVC_REDIST_ROOT STREQUAL "")
            list(
                APPEND
                EISENHOWER_MODELO_PLUGIN_MSVC_RUNTIME_COMMANDS
                COMMAND "${CMAKE_COMMAND}"
                    "-DMODELO_RUNTIME_DIR=$<TARGET_FILE_DIR:ModeloPlugin>"
                    "-DMSVC_REDIST_ROOT=${EISENHOWER_MSVC_REDIST_ROOT}"
                    "-DMSVC_RUNTIME_ARCH=${EISENHOWER_MSVC_RUNTIME_ARCH}"
                    "-DCONFIG=Release"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modelo_msvc_runtime.cmake"
            )
        endif()
        add_custom_target(
            ModeloPluginDeploy
            ALL
            DEPENDS ModeloPlugin
            ${EISENHOWER_MODELO_PLUGIN_DEPLOY_COMMANDS}
            COMMAND "${CMAKE_COMMAND}"
                "-DMODELS_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/models"
                "-DCPU_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/llama-b8987-bin-win-cpu-x64"
                "-DVULKAN_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/modelo/metodoModelo/qwen/llama-b8987-bin-win-vulkan-x64"
                "-DDEST_DIR=$<TARGET_FILE_DIR:ModeloPlugin>"
                "-DEMBED_GGUF_MODEL=${EISENHOWER_EMBED_GGUF_MODEL}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_modelo_runtime.cmake"
            ${EISENHOWER_MODELO_PLUGIN_MSVC_RUNTIME_COMMANDS}
            COMMENT "Deploying optional ModeloPlugin with Qwen GGUF and llama.cpp runtimes"
            VERBATIM
        )
        eisenhower_chain_deploy_target(ModeloPluginDeploy)
    endif()
endif()

if(EISENHOWER_BUILD_AUDIOLIVRO_PLUGIN)
    add_library(AudiolivroPlugin MODULE
        ${EISENHOWER_AUDIOLIVRO_PLUGIN_SOURCES}
        ${EISENHOWER_SHARED_UTIL_SOURCES}
    )

    target_include_directories(AudiolivroPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(AudiolivroPlugin PRIVATE QT_PLUGIN EISENHOWER_BUILD_AUDIOLIVRO_PLUGIN_DLL=1)

    target_link_libraries(AudiolivroPlugin
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Multimedia
            Qt6::Network
            Qt6::Pdf
            Qt6::PdfWidgets
            Qt6::WebSockets
            Qt6::Widgets
            Advapi32
            User32
    )

    eisenhower_apply_performance_options(AudiolivroPlugin)
    eisenhower_disable_msvc_plugin_import_library(AudiolivroPlugin)

    set(EISENHOWER_AUDIOLIVRO_PLUGIN_DIR "${CMAKE_BINARY_DIR}/plugins/audiolivro")
    file(MAKE_DIRECTORY "${EISENHOWER_AUDIOLIVRO_PLUGIN_DIR}")
    set_target_properties(
        AudiolivroPlugin
        PROPERTIES
            OUTPUT_NAME "AudiolivroPlugin"
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${EISENHOWER_AUDIOLIVRO_PLUGIN_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EISENHOWER_AUDIOLIVRO_PLUGIN_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${EISENHOWER_AUDIOLIVRO_PLUGIN_DIR}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${CONFIG}" CONFIG_UPPER)
            set_target_properties(
                AudiolivroPlugin
                PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_AUDIOLIVRO_PLUGIN_DIR}"
                    RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_AUDIOLIVRO_PLUGIN_DIR}"
                    ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_AUDIOLIVRO_PLUGIN_DIR}"
            )
        endforeach()
    endif()

    if(WIN32)
        get_filename_component(QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
        set(WINDEPLOYQT_EXECUTABLE "${QT6_PREFIX_DIR}/bin/windeployqt.exe")
        set(QTPATHS_EXECUTABLE "${QT6_PREFIX_DIR}/bin/qtpaths6.exe")

        eisenhower_prepare_windows_plugin_deploy_commands(
            EISENHOWER_AUDIOLIVRO_PLUGIN_DEPLOY_COMMANDS
            AudiolivroPlugin
            "${WINDEPLOYQT_EXECUTABLE}"
            "${QTPATHS_EXECUTABLE}"
        )
        add_custom_target(
            AudiolivroPluginDeploy
            ALL
            DEPENDS AudiolivroPlugin
            ${EISENHOWER_AUDIOLIVRO_PLUGIN_DEPLOY_COMMANDS}
            COMMAND "${CMAKE_COMMAND}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations"
                "-DDEST_DIR=$<TARGET_FILE_DIR:AudiolivroPlugin>/translations"
                "-DFILE_PATTERN=leitor_*.qm"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_qm_translations.cmake"
            COMMENT "Deploying AudiolivroPlugin package without duplicated app runtime"
            VERBATIM
        )
        eisenhower_chain_deploy_target(AudiolivroPluginDeploy)
    endif()
endif()

if(EISENHOWER_BUILD_AGENDA_PLUGIN)
    add_library(AgendaPlugin MODULE
        ${EISENHOWER_AGENDA_PLUGIN_SOURCES}
        ${EISENHOWER_SHARED_UTIL_SOURCES}
    )

    target_include_directories(AgendaPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(AgendaPlugin PRIVATE QT_PLUGIN EISENHOWER_BUILD_AGENDA_PLUGIN_DLL=1)

    target_link_libraries(AgendaPlugin
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Sql
            Qt6::Widgets
            Advapi32
            User32
    )

    eisenhower_apply_performance_options(AgendaPlugin)
    eisenhower_disable_msvc_plugin_import_library(AgendaPlugin)

    set(EISENHOWER_AGENDA_PLUGIN_DIR "${CMAKE_BINARY_DIR}/plugins/agenda")
    file(MAKE_DIRECTORY "${EISENHOWER_AGENDA_PLUGIN_DIR}")
    set_target_properties(
        AgendaPlugin
        PROPERTIES
            OUTPUT_NAME "AgendaPlugin"
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${EISENHOWER_AGENDA_PLUGIN_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EISENHOWER_AGENDA_PLUGIN_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${EISENHOWER_AGENDA_PLUGIN_DIR}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${CONFIG}" CONFIG_UPPER)
            set_target_properties(
                AgendaPlugin
                PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_AGENDA_PLUGIN_DIR}"
                    RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_AGENDA_PLUGIN_DIR}"
                    ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_AGENDA_PLUGIN_DIR}"
            )
        endforeach()
    endif()

    if(WIN32)
        get_filename_component(QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
        set(WINDEPLOYQT_EXECUTABLE "${QT6_PREFIX_DIR}/bin/windeployqt.exe")
        set(QTPATHS_EXECUTABLE "${QT6_PREFIX_DIR}/bin/qtpaths6.exe")

        eisenhower_prepare_windows_plugin_deploy_commands(
            EISENHOWER_AGENDA_PLUGIN_DEPLOY_COMMANDS
            AgendaPlugin
            "${WINDEPLOYQT_EXECUTABLE}"
            "${QTPATHS_EXECUTABLE}"
        )
        add_custom_target(
            AgendaPluginDeploy
            ALL
            DEPENDS AgendaPlugin
            ${EISENHOWER_AGENDA_PLUGIN_DEPLOY_COMMANDS}
            COMMAND "${CMAKE_COMMAND}" -E rm -rf
                "$<TARGET_FILE_DIR:AgendaPlugin>/assets"
            COMMAND "${CMAKE_COMMAND}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations"
                "-DDEST_DIR=$<TARGET_FILE_DIR:AgendaPlugin>/translations"
                "-DFILE_PATTERN=agenda_*.qm"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_qm_translations.cmake"
            COMMENT "Deploying AgendaPlugin package without duplicated app runtime"
            VERBATIM
        )
        eisenhower_chain_deploy_target(AgendaPluginDeploy)
    endif()
endif()

if(EISENHOWER_BUILD_POMODORO_PLUGIN)
    add_library(PomodoroPlugin MODULE
        ${EISENHOWER_POMODORO_PLUGIN_SOURCES}
        ${EISENHOWER_SHARED_UTIL_SOURCES}
        source/ui/ui_13_RotatedTabButton.cpp
    )

    target_include_directories(PomodoroPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(PomodoroPlugin PRIVATE QT_PLUGIN EISENHOWER_BUILD_POMODORO_PLUGIN_DLL=1)

    target_link_libraries(PomodoroPlugin
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Multimedia
            Qt6::Widgets
            Advapi32
            User32
    )

    eisenhower_apply_performance_options(PomodoroPlugin)
    eisenhower_disable_msvc_plugin_import_library(PomodoroPlugin)

    set(EISENHOWER_POMODORO_PLUGIN_DIR "${CMAKE_BINARY_DIR}/plugins/pomodoro")
    file(MAKE_DIRECTORY "${EISENHOWER_POMODORO_PLUGIN_DIR}")
    set_target_properties(
        PomodoroPlugin
        PROPERTIES
            OUTPUT_NAME "PomodoroPlugin"
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${EISENHOWER_POMODORO_PLUGIN_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EISENHOWER_POMODORO_PLUGIN_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${EISENHOWER_POMODORO_PLUGIN_DIR}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${CONFIG}" CONFIG_UPPER)
            set_target_properties(
                PomodoroPlugin
                PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_POMODORO_PLUGIN_DIR}"
                    RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_POMODORO_PLUGIN_DIR}"
                    ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_POMODORO_PLUGIN_DIR}"
            )
        endforeach()
    endif()

    if(WIN32)
        get_filename_component(QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
        set(WINDEPLOYQT_EXECUTABLE "${QT6_PREFIX_DIR}/bin/windeployqt.exe")
        set(QTPATHS_EXECUTABLE "${QT6_PREFIX_DIR}/bin/qtpaths6.exe")

        eisenhower_prepare_windows_plugin_deploy_commands(
            EISENHOWER_POMODORO_PLUGIN_DEPLOY_COMMANDS
            PomodoroPlugin
            "${WINDEPLOYQT_EXECUTABLE}"
            "${QTPATHS_EXECUTABLE}"
        )
        add_custom_target(
            PomodoroPluginDeploy
            ALL
            DEPENDS PomodoroPlugin
            ${EISENHOWER_POMODORO_PLUGIN_DEPLOY_COMMANDS}
            COMMAND "${CMAKE_COMMAND}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations"
                "-DDEST_DIR=$<TARGET_FILE_DIR:PomodoroPlugin>/translations"
                "-DFILE_PATTERN=pomodoro_*.qm"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_qm_translations.cmake"
            COMMAND "${CMAKE_COMMAND}"
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/modules/pomodoro/Pomodoro/Alarme/sound
                -DDEST_DIR=$<TARGET_FILE_DIR:PomodoroPlugin>/sound
                -DCLEAN_DESTINATION=ON
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_optional_directory.cmake"
            COMMAND "${CMAKE_COMMAND}" -E rm -rf
                "$<TARGET_FILE_DIR:PomodoroPlugin>/Alarme"
            COMMENT "Deploying PomodoroPlugin package without duplicated app runtime"
            VERBATIM
        )
        eisenhower_chain_deploy_target(PomodoroPluginDeploy)
    endif()
endif()

if(EISENHOWER_BUILD_GANTT_PLUGIN)
    add_library(GanttPlugin MODULE
        ${EISENHOWER_GANTT_PLUGIN_SOURCES}
        ${EISENHOWER_SHARED_UTIL_SOURCES}
    )

    target_include_directories(GanttPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(GanttPlugin PRIVATE QT_PLUGIN EISENHOWER_BUILD_GANTT_PLUGIN_DLL=1)

    target_link_libraries(GanttPlugin
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Network
            Qt6::Sql
            Qt6::Widgets
            Advapi32
            User32
    )

    eisenhower_apply_performance_options(GanttPlugin)
    eisenhower_disable_msvc_plugin_import_library(GanttPlugin)

    set(EISENHOWER_GANTT_PLUGIN_DIR "${CMAKE_BINARY_DIR}/plugins/gantt")
    file(MAKE_DIRECTORY "${EISENHOWER_GANTT_PLUGIN_DIR}")
    set_target_properties(
        GanttPlugin
        PROPERTIES
            OUTPUT_NAME "GanttPlugin"
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${EISENHOWER_GANTT_PLUGIN_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EISENHOWER_GANTT_PLUGIN_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${EISENHOWER_GANTT_PLUGIN_DIR}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${CONFIG}" CONFIG_UPPER)
            set_target_properties(
                GanttPlugin
                PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_GANTT_PLUGIN_DIR}"
                    RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_GANTT_PLUGIN_DIR}"
                    ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${EISENHOWER_GANTT_PLUGIN_DIR}"
            )
        endforeach()
    endif()

    if(WIN32)
        get_filename_component(QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
        set(WINDEPLOYQT_EXECUTABLE "${QT6_PREFIX_DIR}/bin/windeployqt.exe")
        set(QTPATHS_EXECUTABLE "${QT6_PREFIX_DIR}/bin/qtpaths6.exe")

        eisenhower_prepare_windows_plugin_deploy_commands(
            EISENHOWER_GANTT_PLUGIN_DEPLOY_COMMANDS
            GanttPlugin
            "${WINDEPLOYQT_EXECUTABLE}"
            "${QTPATHS_EXECUTABLE}"
        )
        add_custom_target(
            GanttPluginDeploy
            ALL
            DEPENDS GanttPlugin
            ${EISENHOWER_GANTT_PLUGIN_DEPLOY_COMMANDS}
            COMMAND "${CMAKE_COMMAND}" -E rm -rf
                "$<TARGET_FILE_DIR:GanttPlugin>/assets"
            COMMAND "${CMAKE_COMMAND}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations"
                "-DDEST_DIR=$<TARGET_FILE_DIR:GanttPlugin>/translations"
                "-DFILE_PATTERN=languagegantt_*.qm"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_qm_translations.cmake"
            COMMENT "Deploying GanttPlugin package without duplicated app runtime"
            VERBATIM
        )
        eisenhower_chain_deploy_target(GanttPluginDeploy)
    endif()
endif()

if(EISENHOWER_BUILD_TESTS)
    add_subdirectory(tests)
endif()

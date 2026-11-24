cmake_minimum_required(VERSION 3.28)

if(POLICY CMP0138)
    cmake_policy(SET CMP0138 NEW)
endif()

file(TO_CMAKE_PATH "${CMAKE_BINARY_DIR}" _eisenhower_normalized_binary_dir)
if(_eisenhower_normalized_binary_dir MATCHES "/build/build_tests_(mingw|msvc)/?$")
    message(FATAL_ERROR "As builds legadas build_tests_mingw e build_tests_msvc foram descontinuadas e integradas em build_suite_testes_mingw e build_suite_testes_msvc.")
endif()

# Shared static library containing all host, module, UI, model, and utility code (excluding main.cpp).
# No external distribution is performed for test targets.
set(EISENHOWER_TEST_CORE_SOURCES)
foreach(_src IN LISTS EISENHOWER_HOST_LIB_SOURCES)
    if(IS_ABSOLUTE "${_src}")
        list(APPEND EISENHOWER_TEST_CORE_SOURCES "${_src}")
    else()
        list(APPEND EISENHOWER_TEST_CORE_SOURCES "${CMAKE_SOURCE_DIR}/${_src}")
    endif()
endforeach()

add_library(EisenhowerTestCore STATIC
    ${EISENHOWER_TEST_CORE_SOURCES}
)

target_include_directories(EisenhowerTestCore
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(EisenhowerTestCore
    PUBLIC
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

eisenhower_apply_performance_options(EisenhowerTestCore)

if(MSVC)
    target_compile_options(EisenhowerTestCore PRIVATE /bigobj)
endif()

get_filename_component(EISENHOWER_TEST_QT6_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)

set(EISENHOWER_TEST_TARGETS)

function(eisenhower_add_qt_test TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            EisenhowerTestCore
            Qt6::Test
    )

    target_include_directories(${TARGET_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_definitions(${TARGET_NAME}
        PRIVATE
            EISENHOWER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
            EISENHOWER_BUILD_DIR="${CMAKE_BINARY_DIR}"
            EISENHOWER_TOOLCHAIN_SUFFIX="${EISENHOWER_BUILD_TOOLCHAIN_SUFFIX}"
    )

    eisenhower_apply_performance_options(${TARGET_NAME})

    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /bigobj)
    endif()

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)

    set_tests_properties(${TARGET_NAME} PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        TIMEOUT 120
    )

    if(WIN32)
        set(_test_env_mod "PATH=path_list_prepend:${EISENHOWER_TEST_QT6_PREFIX_DIR}/bin")
        if(MINGW AND EISENHOWER_MINGW_BIN_DIR)
            list(APPEND _test_env_mod "PATH=path_list_prepend:${EISENHOWER_MINGW_BIN_DIR}")
        endif()

        set_tests_properties(${TARGET_NAME} PROPERTIES
            ENVIRONMENT
                "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${EISENHOWER_TEST_QT6_PREFIX_DIR}/plugins;QT_LOGGING_RULES=qt.qpa.plugin=false"
            ENVIRONMENT_MODIFICATION
                "${_test_env_mod}"
        )
    else()
        set_tests_properties(${TARGET_NAME} PROPERTIES
            ENVIRONMENT
                "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${EISENHOWER_TEST_QT6_PREFIX_DIR}/plugins"
        )
    endif()

    set(EISENHOWER_TEST_TARGETS ${EISENHOWER_TEST_TARGETS} ${TARGET_NAME} PARENT_SCOPE)
endfunction()

eisenhower_add_qt_test(test_core_utils          test_core_utils.cpp)
eisenhower_add_qt_test(test_store_models        test_store_models.cpp)
eisenhower_add_qt_test(test_ui_host             test_ui_host.cpp)
eisenhower_add_qt_test(test_plugin_pomodoro     test_plugin_pomodoro.cpp)
eisenhower_add_qt_test(test_plugin_feynman      test_plugin_feynman.cpp)
eisenhower_add_qt_test(test_plugin_mapamental   test_plugin_mapamental.cpp)
eisenhower_add_qt_test(test_plugin_modelo       test_plugin_modelo.cpp)
eisenhower_add_qt_test(test_plugin_audiolivro   test_plugin_audiolivro.cpp)
eisenhower_add_qt_test(test_plugin_agenda       test_plugin_agenda.cpp)
eisenhower_add_qt_test(test_plugin_gantt        test_plugin_gantt.cpp)
eisenhower_add_qt_test(test_frameworks_plugins  test_frameworks_plugins.cpp)

add_custom_target(EisenhowerTestSuite
    DEPENDS ${EISENHOWER_TEST_TARGETS}
    COMMENT "Built complete Eisenhower / Lumen test suite (no external distribution)"
)

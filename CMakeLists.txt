cmake_minimum_required(VERSION 3.24)
project(InvokeInvoiceSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Widgets HttpServer Network Test REQUIRED)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/forms")

# Shared invoice models and PDF generation, also used by the offline frontend.
# This library must not depend on the database or the HTTP server.
add_library(InvoiceCore STATIC
    src/Domain/Accounts/User.cpp
    src/Infrastructure/Pdf/InvoicePdfGenerator.cpp
)
target_include_directories(InvoiceCore PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(InvoiceCore PUBLIC Qt6::Core fmt::fmt unofficial::libharu::hpdf)

# Desktop pages, local session state and the HTTP client.
file(GLOB FRONTEND_VIEW_SOURCES CONFIGURE_DEPENDS
    src/View/UICode/Views/*.cpp
    include/View/UICode/Views/*.h
)
add_library(InvoiceFrontend STATIC
    ${FRONTEND_VIEW_SOURCES}
    src/View/MainWindow.cpp
    include/View/MainWindow.h
    src/Application/Controllers/AppController.cpp
    include/Application/Controllers/AppController.h
    src/Application/Accounts/FrontendAccountSession.cpp
    include/Application/Accounts/FrontendAccountSession.h
    src/Application/Invoices/OfflineInvoiceSession.cpp
    src/Infrastructure/Http/ApiClient.cpp
    include/Infrastructure/Http/ApiClient.h
)
target_link_libraries(InvoiceFrontend PUBLIC InvoiceCore Qt6::Widgets Qt6::Network)

qt_add_executable(InvokeInvoiceSystem
    src/View/InvokeInvoiceSystem.cpp
    forms/Invoice_Resources.qrc
)
target_link_libraries(InvokeInvoiceSystem PRIVATE InvoiceFrontend)
set_target_properties(InvokeInvoiceSystem PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE ON)

# Only the backend owns MongoDB, password verification and the HTTP server.
add_library(passlib STATIC
    src/Infrastructure/Security/PasswordHashing/bcrypt.cpp
    src/Infrastructure/Security/PasswordHashing/blowfish.cpp
)
target_include_directories(passlib PUBLIC "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/Infrastructure/Security/PasswordHashing")

add_library(InvoiceBackend STATIC
    src/Application/Accounts/AccountManager.cpp
    src/Application/Accounts/SetUserFromDB.cpp
    src/Application/Business/SetBusinessFromDB.cpp
    src/Infrastructure/Database/Mongo/MongoDBHandler.cpp
    src/Infrastructure/Database/Mongo/MongoDBDataManager.cpp
    src/Infrastructure/Database/Services/AccountServices.cpp
    src/Infrastructure/Database/Services/InvoiceServices.cpp
    src/Infrastructure/Http/FakeServer.cpp
)
target_link_libraries(InvoiceBackend PUBLIC
    InvoiceCore Qt6::HttpServer Qt6::Network Qt6::Gui
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    PRIVATE passlib
)

qt_add_executable(InvokeInvoiceBackend src/Backend/InvokeInvoiceBackend.cpp)
target_link_libraries(InvokeInvoiceBackend PRIVATE InvoiceBackend)
set_target_properties(InvokeInvoiceBackend PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)

# Tests use the same libraries as the executables, without either application's main().
enable_testing()
qt_add_executable(BusinessSessionTests tests/BusinessSessionTests.cpp forms/Invoice_Resources.qrc)
target_link_libraries(BusinessSessionTests PRIVATE InvoiceFrontend Qt6::Test Qt6::HttpServer)
add_test(NAME BusinessSession COMMAND BusinessSessionTests)
set_tests_properties(BusinessSession PROPERTIES LABELS "ui;session" TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_ASSUME_STDERR_HAS_CONSOLE=1")

qt_add_executable(FrontendSessionTests tests/FrontendSessionTests.cpp forms/Invoice_Resources.qrc)
target_link_libraries(FrontendSessionTests PRIVATE InvoiceFrontend Qt6::Test Qt6::HttpServer)
add_test(NAME FrontendSession COMMAND FrontendSessionTests)
set_tests_properties(FrontendSession PROPERTIES LABELS "ui;session" TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_ASSUME_STDERR_HAS_CONSOLE=1")

qt_add_executable(BackendServerTests tests/BackendServerTests.cpp)
target_link_libraries(BackendServerTests PRIVATE InvoiceBackend Qt6::Test)
add_test(NAME BackendServer COMMAND BackendServerTests)
set_tests_properties(BackendServer PROPERTIES LABELS "backend;session" TIMEOUT 30
    ENVIRONMENT "QT_ASSUME_STDERR_HAS_CONSOLE=1")

qt_add_executable(InvokeInvoiceSystemTests tests/PDFGenTests.cpp)
target_link_libraries(InvokeInvoiceSystemTests PRIVATE InvoiceCore Qt6::Test Qt6::Widgets)
target_include_directories(InvokeInvoiceSystemTests PRIVATE "${CMAKE_SOURCE_DIR}/tests")

# Live, read-only MongoDB checks remain opt-in when selecting tests with CTest.
qt_add_executable(DatabaseTests tests/DatabaseTests.cpp)
target_link_libraries(DatabaseTests PRIVATE InvoiceBackend Qt6::Test)
add_test(NAME DatabaseHealth COMMAND DatabaseTests)
set_tests_properties(DatabaseHealth PROPERTIES LABELS "database;integration" TIMEOUT 60
    ENVIRONMENT "QT_ASSUME_STDERR_HAS_CONSOLE=1")

foreach(test_target BusinessSessionTests FrontendSessionTests BackendServerTests InvokeInvoiceSystemTests DatabaseTests)
    set_target_properties(${test_target} PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
endforeach()

# Both processes read logos; JPEG support is needed by the backend as well as the UI.
if(WIN32)
    find_package(libjpeg-turbo CONFIG REQUIRED)
    foreach(invoice_target InvokeInvoiceSystem InvokeInvoiceBackend BusinessSessionTests FrontendSessionTests BackendServerTests InvokeInvoiceSystemTests)
        add_custom_command(TARGET ${invoice_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${invoice_target}>/imageformats"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:Qt6::QJpegPlugin>" "$<TARGET_FILE_DIR:${invoice_target}>/imageformats/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:libjpeg-turbo::jpeg>" "$<TARGET_FILE_DIR:${invoice_target}>/"
            VERBATIM
        )
    endforeach()
endif()

foreach(app_target InvokeInvoiceSystem InvokeInvoiceBackend)
    if(app_target STREQUAL "InvokeInvoiceSystem")
        set(app_component Frontend)
    else()
        set(app_component Backend)
    endif()
    install(TARGETS ${app_target}
        RUNTIME_DEPENDENCY_SET ${app_component}Runtime
        BUNDLE DESTINATION . COMPONENT ${app_component}
        RUNTIME DESTINATION bin COMPONENT ${app_component})
    # windeployqt deploys Qt itself and plugins, but not all third-party DLLs.
    # Scan each executable separately so the desktop package excludes MongoDB.
    if(WIN32)
        install(RUNTIME_DEPENDENCY_SET ${app_component}Runtime
            PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
            POST_EXCLUDE_REGEXES ".*[/\\\\][Ss][Yy][Ss][Tt][Ee][Mm]32[/\\\\].*"
            DIRECTORIES "$<TARGET_FILE_DIR:${app_target}>"
            RUNTIME DESTINATION bin COMPONENT ${app_component})
        install(FILES "$<TARGET_FILE:libjpeg-turbo::jpeg>" DESTINATION bin COMPONENT ${app_component})
    endif()
    qt_generate_deploy_app_script(TARGET ${app_target} OUTPUT_SCRIPT deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
    install(SCRIPT ${deploy_script} COMPONENT ${app_component})
endforeach()

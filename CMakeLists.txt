    cmake_minimum_required(VERSION 3.24)

    project(InvokeInvoiceSystem LANGUAGES CXX)

    file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" INVOICE_MANIFEST)
    string(JSON INVOICE_BASE_VERSION GET "${INVOICE_MANIFEST}" version-string)
    set(INVOICE_APP_VERSION "${INVOICE_BASE_VERSION}" CACHE STRING "Application release version")
    if(NOT INVOICE_APP_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
        message(FATAL_ERROR "INVOICE_APP_VERSION must be major.minor.patch")
    endif()
    add_compile_definitions(INVOKE_APP_VERSION="${INVOICE_APP_VERSION}")

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # If Qt isn't found via PATH/CMAKE_PREFIX_PATH, keep this hint:
    # set(Qt6_DIR "C:/Qt/6.9.1/msvc2022_64/lib/cmake/Qt6")

    # Release configs you use
    set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
    # ---------- Dependencies ----------
    find_package(fmt                CONFIG REQUIRED)
    find_package(bsoncxx            CONFIG REQUIRED)
    find_package(mongocxx           CONFIG REQUIRED)
    find_package(unofficial-libharu CONFIG REQUIRED)
    find_package(Iconv              REQUIRED)
    find_package(Qt6 COMPONENTS Widgets HttpServer Network Test REQUIRED)
    find_package(Qt6Keychain CONFIG REQUIRED)
    qt_standard_project_setup()  # enables AUTOMOC/AUTORCC/AUTOUIC
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    # Let AUTOUIC find ui files under /forms
    set(CMAKE_AUTOUIC_SEARCH_PATHS
        ${CMAKE_AUTOUIC_SEARCH_PATHS}
        ${CMAKE_SOURCE_DIR}/forms
    )

    # ---------- passlib static ----------
    add_library(passlib STATIC
        src/Infrastructure/Security/PasswordHashing/bcrypt.cpp
        src/Infrastructure/Security/PasswordHashing/blowfish.cpp)
        target_include_directories(passlib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Infrastructure/Security/PasswordHashing
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(arc4random_buf "stdlib.h" INVOKE_HAVE_SYSTEM_ARC4RANDOM_BUF)
    if(INVOKE_HAVE_SYSTEM_ARC4RANDOM_BUF)
        target_compile_definitions(passlib PRIVATE INVOKE_HAVE_SYSTEM_ARC4RANDOM_BUF)
    endif()

    # ---------- Auto file pickup ----------
    # Define globs FIRST
    file(GLOB_RECURSE PROJECT_CPP CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/*.cpp
    )
    file(GLOB_RECURSE PROJECT_HDR CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/include/*.h
    )
    file(GLOB_RECURSE PROJECT_UI CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/forms/*.ui
    )
    set(APP_MAIN_CPP ${CMAKE_SOURCE_DIR}/src/View/InvokeInvoiceSystem.cpp)
    set(BACKEND_MAIN_CPP ${CMAKE_SOURCE_DIR}/src/Backend/main.cpp)
    # Each executable must have exactly one entry point.
    list(REMOVE_ITEM PROJECT_CPP ${BACKEND_MAIN_CPP} ${CMAKE_SOURCE_DIR}/src/Backend/InvokeInvoiceBackend.cpp)
    set(TEST_SOURCES ${PROJECT_CPP})
    list(REMOVE_ITEM TEST_SOURCES ${APP_MAIN_CPP})

    # ---------- Headless HTTP backend (Docker / Heroku) ----------
    set(BACKEND_SOURCES
        src/Application/Accounts/AccountManager.cpp
        src/Application/Accounts/SetUserFromDB.cpp
        src/Application/Business/SetBusinessFromDB.cpp
        src/Domain/Accounts/User.cpp
        src/Infrastructure/Database/Mongo/MongoDBHandler.cpp
        src/Infrastructure/Database/Mongo/MongoDBDataManager.cpp
        src/Infrastructure/Database/Services/AccountServices.cpp
        src/Infrastructure/Database/Services/InvoiceServices.cpp
        src/Infrastructure/Http/FakeServer.cpp
        src/Infrastructure/Pdf/InvoicePdfGenerator.cpp
    )
    qt_add_executable(InvokeInvoiceBackend ${BACKEND_MAIN_CPP} ${BACKEND_SOURCES})
    set_target_properties(InvokeInvoiceBackend PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
    target_include_directories(InvokeInvoiceBackend PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(InvokeInvoiceBackend PRIVATE
        passlib
        Qt6::Core Qt6::Gui Qt6::HttpServer Qt6::Network
        fmt::fmt
        unofficial::libharu::hpdf
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    )

    # ---------- App executable ----------
    qt_add_executable(InvokeInvoiceSystem MANUAL_FINALIZATION
        ${PROJECT_CPP}         # must include src/View/InvokeInvoiceSystem.cpp with main()
        ${PROJECT_UI}
    )

    # Register headers so IntelliSense and AUTOMOC can see them
    target_sources(InvokeInvoiceSystem PRIVATE
        ${PROJECT_HDR}
    )

    # Qt resources
    set(CMAKE_AUTORCC ON)
    set_target_properties(InvokeInvoiceSystem PROPERTIES AUTORCC ON)
    target_sources(InvokeInvoiceSystem PRIVATE forms/Invoice_Resources.qrc)

    # Public include root
    target_include_directories(InvokeInvoiceSystem PUBLIC ${CMAKE_SOURCE_DIR}/include)

    target_link_libraries(InvokeInvoiceSystem PRIVATE
        passlib
        Qt6::Widgets
        # Qt6::Pdf
        fmt::fmt 
        Iconv::Iconv
        unofficial::libharu::hpdf
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
        Qt6Keychain::Qt6Keychain
        Qt6::HttpServer
        Qt6::Network
    )

    set_target_properties(InvokeInvoiceSystem PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE   ON
    )
    target_precompile_headers(InvokeInvoiceSystem PRIVATE include/pch.h)

    # ---------- Test executable ----------

    qt_add_executable(InvokeInvoiceSystemTests MANUAL_FINALIZATION
        tests/PDFGenTests.cpp
        src/Domain/Accounts/User.cpp
        src/Infrastructure/Pdf/InvoicePdfGenerator.cpp
    )
    target_link_libraries(InvokeInvoiceSystemTests PRIVATE
        Qt6::Test
        Qt6::Widgets
        fmt::fmt 
        Iconv::Iconv
        unofficial::libharu::hpdf
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
        Qt6Keychain::Qt6Keychain
        Qt6::HttpServer
        Qt6::Network
    )

    target_include_directories(InvokeInvoiceSystemTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests
    )
    qt_generate_deploy_app_script(
        TARGET InvokeInvoiceSystemTests
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
    qt_finalize_executable(InvokeInvoiceSystem
        src/View/UICode/Views/SignupPage.cpp)
    # Static Qt builds (including Linux/vcpkg) must link the headless platform
    # plugin explicitly for the offscreen CTest environment.
    qt_import_plugins(InvokeInvoiceSystemTests INCLUDE Qt6::QOffscreenIntegrationPlugin)
    qt_finalize_executable(InvokeInvoiceSystemTests)

    # Live, read-only MongoDB health checks use the same environment as the application.
    qt_add_executable(DatabaseTests
        tests/DatabaseTests.cpp
        src/Infrastructure/Database/Mongo/MongoDBHandler.cpp
    )
    set_target_properties(DatabaseTests PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
    target_include_directories(DatabaseTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(DatabaseTests PRIVATE
        Qt6::Test
        fmt::fmt
        unofficial::libharu::hpdf
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    )
    include(CTest)

    qt_add_executable(BackendServerTests tests/BackendServerTests.cpp ${BACKEND_SOURCES})
    target_include_directories(BackendServerTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(BackendServerTests PRIVATE
        passlib Qt6::Test Qt6::Gui Qt6::HttpServer Qt6::Network
        fmt::fmt unofficial::libharu::hpdf
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    )

    qt_add_executable(FrontendSessionTests
        tests/FrontendSessionTests.cpp
        src/Application/Accounts/FrontendAccountSession.cpp
        src/Application/Invoices/OfflineInvoiceSession.cpp
        src/Domain/Accounts/User.cpp
        src/Infrastructure/Http/ApiClient.cpp
        include/Infrastructure/Http/ApiClient.h
        src/Infrastructure/Pdf/InvoicePdfGenerator.cpp
    )
    target_include_directories(FrontendSessionTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(FrontendSessionTests PRIVATE
        Qt6::Test Qt6::Gui Qt6::HttpServer Qt6::Network
        fmt::fmt unofficial::libharu::hpdf
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    )
    qt_add_executable(LoggingTests tests/LoggingTests.cpp)
    set_target_properties(LoggingTests PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
    target_include_directories(LoggingTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(LoggingTests PRIVATE Qt6::Test Qt6::HttpServer Qt6::Network)
    qt_add_executable(BusinessSessionTests
        tests/BusinessSessionTests.cpp
        src/Application/Invoices/OfflineInvoiceSession.cpp
        src/Infrastructure/Pdf/InvoicePdfGenerator.cpp
        src/Domain/Accounts/User.cpp
        src/View/UICode/Views/ItemsPage.cpp
        include/View/UICode/Views/ItemsPage.h
        src/Application/Controllers/AppController.cpp
        include/Application/Controllers/AppController.h
        src/View/UICode/Views/Dashboard.cpp
        src/View/UICode/Views/NewInvoiceStock.cpp
        src/Infrastructure/Http/ApiClient.cpp
        include/View/UICode/Views/Dashboard.h
        include/View/UICode/Views/NewInvoiceStock.h
        include/Infrastructure/Http/ApiClient.h
        forms/Invoice_Resources.qrc
        src/View/MainWindow.cpp
        src/View/UICode/Views/LandingPage.cpp
        src/View/UICode/Views/LoginPage.cpp
        src/View/UICode/Views/SignupPage.cpp
        src/View/UICode/Views/ManagementForm.cpp
        src/View/UICode/Views/NewInvoiceCreation.cpp
        src/View/UICode/Views/BusinessInvoiceChoice.cpp
        src/View/UICode/Views/DefaultInvoiceSettings.cpp
        include/View/UICode/Views/DefaultInvoiceSettings.h
        src/Application/Accounts/FrontendAccountSession.cpp
        include/View/MainWindow.h
        include/View/UICode/Views/LandingPage.h
        include/View/UICode/Views/LoginPage.h
        include/View/UICode/Views/SignupPage.h
        include/View/UICode/Views/ManagementForm.h
        include/View/UICode/Views/NewInvoiceCreation.h
        include/View/UICode/Views/BusinessInvoiceChoice.h
    )
    set_target_properties(BusinessSessionTests PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
    target_include_directories(BusinessSessionTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(BusinessSessionTests PRIVATE
        Qt6::Test Qt6::Widgets Qt6::HttpServer Qt6::Network
        fmt::fmt unofficial::libharu::hpdf
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    )
    qt_import_plugins(BusinessSessionTests INCLUDE Qt6::QOffscreenIntegrationPlugin)

    # Build every executable with one command. Discovery starts each Qt Test runner
    # without executing test bodies or contacting the database.
    set(INVOICE_TEST_TARGETS InvokeInvoiceSystemTests DatabaseTests LoggingTests
        BusinessSessionTests FrontendSessionTests BackendServerTests)
    add_custom_target(all-tests DEPENDS ${INVOICE_TEST_TARGETS})
    function(register_invoice_test name target label)
        set_target_properties(${target} PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
        add_test(NAME ${name} COMMAND ${target})
        add_test(NAME ${name}Discovery COMMAND ${target} -functions)
        set_tests_properties(${name} ${name}Discovery PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_ASSUME_STDERR_HAS_CONSOLE=1")
        set_tests_properties(${name} PROPERTIES TIMEOUT 180 LABELS "${label}")
        set_tests_properties(${name}Discovery PROPERTIES TIMEOUT 30 LABELS "smoke")
    endfunction()
    register_invoice_test(PDFGen InvokeInvoiceSystemTests local)
    register_invoice_test(DatabaseHealth DatabaseTests integration)
    register_invoice_test(Logging LoggingTests local)
    register_invoice_test(BusinessSession BusinessSessionTests local)
    register_invoice_test(FrontendSession FrontendSessionTests local)
    register_invoice_test(BackendServer BackendServerTests local)

    # The migration script may deliberately be absent from this checkout.
    if(EXISTS "${CMAKE_SOURCE_DIR}/scripts/migrate_object_ids.py")
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        add_test(NAME ObjectIds COMMAND ${Python3_EXECUTABLE} -m unittest discover
            -s tests -p test_object_ids.py -v)
        set_tests_properties(ObjectIds PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            TIMEOUT 180 LABELS "integration;python")
    else()
        message(STATUS "ObjectIds tests unavailable: scripts/migrate_object_ids.py is absent")
    endif()

    # JPEG logos need Qt's image-format plugin beside locally built executables.
    if(WIN32)
        find_package(libjpeg-turbo CONFIG REQUIRED)
        foreach(invoice_target InvokeInvoiceBackend InvokeInvoiceSystem ${INVOICE_TEST_TARGETS})
            add_custom_command(TARGET ${invoice_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${invoice_target}>/imageformats"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:Qt6::QJpegPlugin>" "$<TARGET_FILE_DIR:${invoice_target}>/imageformats/"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:libjpeg-turbo::jpeg>" "$<TARGET_FILE_DIR:${invoice_target}>/"
                VERBATIM
            )
        endforeach()
        foreach(test_target ${INVOICE_TEST_TARGETS})
            add_custom_command(TARGET ${test_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${test_target}>/platforms"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:Qt6::QOffscreenIntegrationPlugin>" "$<TARGET_FILE_DIR:${test_target}>/platforms/"
                VERBATIM
            )
        endforeach()
    endif()
foreach(app_target InvokeInvoiceSystem InvokeInvoiceBackend)
    qt_generate_deploy_app_script(TARGET ${app_target} OUTPUT_SCRIPT deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
    install(SCRIPT ${deploy_script})
endforeach()

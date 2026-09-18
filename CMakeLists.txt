    cmake_minimum_required(VERSION 3.10...3.24)

    project(InvokeInvoiceSystem LANGUAGES CXX)

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
    set(TEST_SOURCES ${PROJECT_CPP})
    list(REMOVE_ITEM TEST_SOURCES ${APP_MAIN_CPP})

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
        ${TEST_SOURCES}
    )
    target_sources(InvokeInvoiceSystemTests PRIVATE
        ${PROJECT_HDR}
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
    enable_testing()
    qt_add_executable(BusinessSessionTests
        tests/BusinessSessionTests.cpp
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
    add_test(NAME BusinessSession COMMAND BusinessSessionTests)
    # JPEG logos need Qt's image-format plugin beside locally built executables.
    if(WIN32)
        find_package(libjpeg-turbo CONFIG REQUIRED)
        foreach(invoice_target InvokeInvoiceSystem BusinessSessionTests InvokeInvoiceSystemTests)
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
    set_tests_properties(BusinessSession PROPERTIES
        LABELS "ui;session"
        TIMEOUT 30
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_ASSUME_STDERR_HAS_CONSOLE=1"
    )
    add_test(NAME DatabaseHealth COMMAND DatabaseTests)
    set_tests_properties(DatabaseHealth PROPERTIES
        LABELS "database;integration"
        TIMEOUT 60
        ENVIRONMENT "QT_ASSUME_STDERR_HAS_CONSOLE=1"
    )


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
    qt_add_resources(InvokeInvoiceSystem app_resources
    FILES forms/Invoice_Resources.qrc
    )

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
    qt_finalize_executable(InvokeInvoiceSystem)
    qt_finalize_executable(InvokeInvoiceSystemTests)


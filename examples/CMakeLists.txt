find_package(PkgConfig QUIET)

if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "pkg-config not found!" )
    message(FATAL_ERROR "sudo apt-get install pkg-config, or brew install pkg-config")
endif()

pkg_check_modules(libuv REQUIRED libuv)
pkg_check_modules(libenv QUIET libenv)

if (${Boost_FOUND})
    ###################################
    # Boost
    ###################################

    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(amqpcpp_boost_example libboostasio.cpp)

    add_dependencies(amqpcpp_boost_example amqpcpp)

    target_link_libraries(amqpcpp_boost_example amqpcpp boost_system pthread dl ssl)
endif ()

if (libev_FOUND)
    ###################################
    # Libev
    ###################################

    add_executable(amqpcpp_libev_example libev.cpp)

    add_dependencies(amqpcpp_libev_example amqpcpp)

    target_link_libraries(amqpcpp_libev_example
            amqpcpp
            ev
            pthread
            dl
            ${OPENSSL_SSL_LIBRARY}
            ${OPENSSL_CRYPTO_LIBRARY})
endif ()


if (${libuv_FOUND})
    ###################################
    # Libuv
    ###################################

    include_directories(${libuv_INCLUDEDIR})

    link_directories(${libuv_LIBRARY_DIRS})

    add_executable(amqpcpp_libuv_example libuv.cpp)

    add_dependencies(amqpcpp_libuv_example amqpcpp)

    target_link_libraries(
            amqpcpp_libuv_example
            amqpcpp
            ${libuv_LIBRARIES}
            ${OPENSSL_SSL_LIBRARY}
            ${OPENSSL_CRYPTO_LIBRARY}
    )

endif ()

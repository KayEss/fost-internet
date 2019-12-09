# file(DOWNLOAD https://curl.haxx.se/ca/cacert.pem
#     ${CMAKE_CURRENT_SOURCE_DIR}/cacert.pem TLS_VERIFY ON)
install(FILES
        cacert.pem
    DESTINATION etc/fost)

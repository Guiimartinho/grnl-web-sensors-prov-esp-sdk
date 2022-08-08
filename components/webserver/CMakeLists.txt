idf_component_register(SRCS "teste.c" "webserver.c"
                    INCLUDE_DIRS ".")
                    #PRIV_REQUIRES protobuf-c mbedtls console esp_http_server)
                    #INCLUDE_DIRS "include"
                    #PRIV_INCLUDE_DIRS "src/port/esp32" "src/util"
                    #REQUIRES nghttp # for http_parser.h
                    #PRIV_REQUIRES lwip mbedtls esp_timer)
                    #spiffs_create_partition_image(storage ../../html FLASH_IN_PROJECT)
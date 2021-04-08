file(GLOB SRC_LIST "*.c")
idf_component_register(SRCS ${SRC_LIST}
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES Led Switch Json_parse Http Smartconfig ServerTimer
)


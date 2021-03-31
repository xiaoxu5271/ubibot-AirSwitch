idf_component_register(SRCS ./Json_parse.c
                       INCLUDE_DIRS include                       # Edit following two lines to set component requirements (see docs)
                    #    REQUIRES )
                    REQUIRES  HLW8112 json E2prom Http Bluetooth ota Led my_base64 Switch My_Mqtt )


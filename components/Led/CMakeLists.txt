idf_component_register(SRCS ./Led.c
                       INCLUDE_DIRS include                       # Edit following two lines to set component requirements (see docs)
                    #    REQUIRES )
                       PRIV_REQUIRES Json_parse Smartconfig HLW8112  )


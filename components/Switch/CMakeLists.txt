idf_component_register(SRCS ./Switch.c
                       INCLUDE_DIRS include                       # Edit following two lines to set component requirements (see docs)
                    #    REQUIRES )
                       PRIV_REQUIRES json ServerTimer  Http Json_parse  E2prom  Smartconfig Led  HLW8112  )


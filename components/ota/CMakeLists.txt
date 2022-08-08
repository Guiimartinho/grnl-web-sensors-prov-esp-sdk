idf_component_register(SRCS "ota.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES ${project_dir}/components/ota/server_certs/ca_cert.pem)

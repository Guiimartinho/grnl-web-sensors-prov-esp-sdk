idf_component_register(SRCS "main.c",
                            "modem.c"
                    INCLUDE_DIRS ".")

# Create a SPIFFS image from the contents of the 'font' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash
#spiffs_create_partition_image(storage ../html FLASH_IN_PROJECT)
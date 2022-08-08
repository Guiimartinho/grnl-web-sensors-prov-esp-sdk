# _Sample project_

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This is the simplest buildable example. The example is used by command `idf.py create-project`
that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)



## How to use example
We encourage the users to use the example as a template for the new projects.
A recommended way is to follow the instructions on a [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Example folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.




## https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/linux-setup.html
[command]
sudo chmod -R 777 /dev/ttyUSB0


## mkspiffs
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html
https://www.coldreactor.com/2018/12/29/ESP8266-SPIFFS-Manual.html


## ESP8266 RTOS SDK User Manual
https://readthedocs.com/projects/espressif-esp8266-rtos-sdk/downloads/pdf/latest/


## incorporar arquivo imutavel
https://esp32.com/viewtopic.php?t=1933

## json
https://www.microgenios.com.br/projeto-pratico-com-esp8266-e-sdk-idf-consumindo-um-json/
https://github.com/nopnop2002/esp-idf-json/tree/master/json-http-client


## exemplo web page embarcada
https://github.com/nopnop2002/esp-idf-pwm-slider



## Comandos uteis
# mksppif
em:
user@user/folder:~/mkspiffs$ 

verificar configuração do mkspiffs
./mkspiffs --version

atualmente está assim:
mkspiffs ver. 0.2.3-7-gf248296
Build configuration name: esp-idf
SPIFFS ver. 0.3.7-5-gf5e26c4
Extra build flags: -DSPIFFS_OBJ_META_LEN=0 -DSPIFFS_USE_MAGIC=1 -DSPIFFS_USE_MAGIC_LENGTH=0 -DSPIFFS_ALIGNED_OBJECT_INDEX_TABLES=1 -DSPIFFS_OBJ_NAME_LEN=32
SPIFFS configuration:
  SPIFFS_OBJ_NAME_LEN: 32
  SPIFFS_OBJ_META_LEN: 0
  SPIFFS_USE_MAGIC: 1
  SPIFFS_USE_MAGIC_LENGTH: 0
  SPIFFS_ALIGNED_OBJECT_INDEX_TABLES: 1

para alterar a configuração, tem que editar o arquivo build_all_configs.sh na pasta do mkspiffs
build mksppif:
./build_all_configs.sh

gerar binário das pastas
./mkspiffs -c ../esp/Projetos/greenole/html/ -b 4096 -p 256 -s 0xf0000 spiffs.bin

flash do binario na partição
esptool.py --chip esp8266 --port /dev/ttyUSB0 --baud 921600 write_flash -z 0x2DA000 spiffs.bin


# compilador
na pasta:
user@user/folder:~/esp/Projetos/greenole$ 

- liberar porta usb para flash
sudo chmod -R 777 /dev/ttyUSB0

- compilar e gravar na esp
make flash monitor

- apagar a flash e suas partições, compilar e gravar na esp (necessário gravar os dados das partições após esse procedimento)
make erase_flash flash monitor

- exportar os path's de configuração (caso necessário pois já foram inseridos no ~/.profile)
export IDF_PATH=~/esp/ESP8266_RTOS_SDK
export PATH="$HOME/esp/xtensa-lx106-elf/bin/:$PATH"#grnl-user

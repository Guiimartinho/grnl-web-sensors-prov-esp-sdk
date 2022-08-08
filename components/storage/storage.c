///* SPIFFS filesystem example.
//   This example code is in the Public Domain (or CC0 licensed, at your option.)
//
//   Unless required by applicable law or agreed to in writing, this
//   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.
//*/
//
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "mbedtls/md5.h"

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/event_groups.h"
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/sockets.h>

#include "esp_vfs_semihost.h"
#include "esp_vfs_fat.h"

#include "cJSON.h"
#include "define.h"

static const char *TAG = "spiffs";
//
//void app_main(void)
//{
//    ESP_LOGI(TAG, "Initializing SPIFFS");
//    
//    esp_vfs_spiffs_conf_t conf = {
//      .base_path = "/spiffs",
//      .partition_label = NULL,
//      .max_files = 5,
//      .format_if_mount_failed = true
//    };
//    
//    // Use settings defined above to initialize and mount SPIFFS filesystem.
//    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
//    esp_err_t ret = esp_vfs_spiffs_register(&conf);
//
//    if (ret != ESP_OK) {
//        if (ret == ESP_FAIL) {
//            ESP_LOGE(TAG, "Failed to mount or format filesystem");
//        } else if (ret == ESP_ERR_NOT_FOUND) {
//            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
//        } else {
//            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
//        }
//        return;
//    }
//    
//    size_t total = 0, used = 0;
//    ret = esp_spiffs_info(NULL, &total, &used);
//    if (ret != ESP_OK) {
//        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
//    } else {
//        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
//    }
//
//    // Use POSIX and C standard library functions to work with files.
//    // First create a file.
//    ESP_LOGI(TAG, "Opening file");
//    FILE* f = fopen("/spiffs/hello.txt", "w");
//    if (f == NULL) {
//        ESP_LOGE(TAG, "Failed to open file for writing");
//        return;
//    }
//    fprintf(f, "Hello World!\n");
//    fclose(f);
//    ESP_LOGI(TAG, "File written");
//
//    // Check if destination file exists before renaming
//    struct stat st;
//    if (stat("/spiffs/foo.txt", &st) == 0) {
//        // Delete it if it exists
//        unlink("/spiffs/foo.txt");
//    }
//
//    // Rename original file
//    ESP_LOGI(TAG, "Renaming file");
//    if (rename("/spiffs/hello.txt", "/spiffs/foo.txt") != 0) {
//        ESP_LOGE(TAG, "Rename failed");
//        return;
//    }
//
//    // Open renamed file for reading
//    ESP_LOGI(TAG, "Reading file");
//    f = fopen("/spiffs/foo.txt", "r");
//    if (f == NULL) {
//        ESP_LOGE(TAG, "Failed to open file for reading");
//        return;
//    }
//    char line[64];
//    fgets(line, sizeof(line), f);
//    fclose(f);
//    // strip newline
//    char* pos = strchr(line, '\n');
//    if (pos) {
//        *pos = '\0';
//    }
//    ESP_LOGI(TAG, "Read from file: '%s'", line);
//
//    // All done, unmount partition and disable SPIFFS
//    esp_vfs_spiffs_unregister(NULL);
//    ESP_LOGI(TAG, "SPIFFS unmounted");
//}
//
static void SPIFFS_Directory(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(TAG,"d_name=%s/%s d_ino=%d d_type=%x", path, pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}

esp_err_t SPIFFS_Mount(char * path, char * label, int max_files) {
	esp_vfs_spiffs_conf_t conf = {
		.base_path = path,
		.partition_label = label,
		.max_files = max_files,
		.format_if_mount_failed =false
	};

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is anall-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret ==ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret== ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return ret;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}

	if (ret == ESP_OK) {
		ESP_LOGI(TAG, "Mount %s to %s success", path, label);
		SPIFFS_Directory(path);
	}
	return ret;
}

void read_index_txt(void)
{
    ESP_LOGI(TAG, "Reading index.txt");

    // Open for reading hello.txt
    FILE* f = fopen("/spiffs/index.html", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open /spiffs/index.html");
		return;
    }
    char buf[64];
    memset(buf, 0, sizeof(buf));
    fread(buf, 1, sizeof(buf), f);
    fclose(f);

    // Display the read contents from the file
    ESP_LOGI(TAG, "Read from index.txt: %s", buf);
}

/* Function to initialize SPIFFS */
esp_err_t init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

	// Initialize SPIFFS
	ESP_LOGI(TAG, "Initializing SPIFFS");
	if (SPIFFS_Mount("/html", "storage", 10) != ESP_OK)
	{
		ESP_LOGE(TAG, "SPIFFS mount failed");
		while(1) { vTaskDelay(1); }
	}

//    esp_vfs_spiffs_conf_t conf = {
//      .base_path = "/spiffs",
//      .partition_label = NULL,
//      .max_files = 5,   // This decides the maximum number of files that can be created on the storage
//      .format_if_mount_failed = true
//    };
//
//    esp_err_t ret = esp_vfs_spiffs_register(&conf);
//    if (ret != ESP_OK) {
//        if (ret == ESP_FAIL) {
//            ESP_LOGE(TAG, "Failed to mount or format filesystem");
//        } else if (ret == ESP_ERR_NOT_FOUND) {
//            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
//        } else {
//            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
//        }
//        return ESP_FAIL;
//    }
//
//    size_t total = 0, used = 0;
//    ret = esp_spiffs_info(NULL, &total, &used);
//    if (ret != ESP_OK) {
//        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
//        return ESP_FAIL;
//    }
//
//    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);


	ESP_LOGI(TAG, "Reading config");
	// Open for reading config.json
	FILE* f = fopen("/html/config.json", "r");
	if (f == NULL) {
	    ESP_LOGE(TAG, "Failed to open config file");
		return 0;	//TODO: criar um arquivo json
	}
	
	char buf[1024];
//	size_t size = f.size();
//	ESP_LOGI(TAG, "Size Config file: %d", size);
//	if (size > sizeof(buf))
//    {
//        ESP_LOGE("Config file is bigger than %d", sizeof(buf));
//        return false;
//    }

	memset(buf, 0, sizeof(buf));
	fread(buf, 1, sizeof(buf), f);
	fclose(f);

	ESP_LOGI(TAG, "Read from Config file: %s", buf);

	root = cJSON_Parse(buf);

	char version[1024];
	cJSON *device_info = cJSON_GetObjectItem(root, "device_info");
	strncpy(version, cJSON_GetObjectItemCaseSensitive(device_info, "hw_version")->valuestring, sizeof(version));
	

	cJSON *config = cJSON_GetObjectItem(root, "config");
	cJSON *network = cJSON_GetObjectItem(config, "network");
	cJSON *wifi_ap = cJSON_GetObjectItem(network, "wifi_ap");
//	strncpy(version, cJSON_GetObjectItemCaseSensitive(device_info, "mask")->valuestring, sizeof(version));


//	ESP_LOGI(TAG, "JSON: hw_version: %s", version);

//	ESP_LOGI(TAG, "Print cJSON file:\n%s",cJSON_Print(root));
//	
//	cJSON_SetValuestring(cJSON_GetObjectItem(device_info, "hw_version"), 15);
//	
//	
//
//
//
//PROJECT_VER
//
//
//esp_get_idf_version()
//
//esp_chip_info(esp_chip_info_t* out_info)

	ESP_LOGI(TAG, "Print cJSON file:\n%s",cJSON_PrintUnformatted(wifi_ap));





    return ESP_OK;
}

/* Declare the function which starts the file server.
 * Implementation of this function is to be found in
 * file_server.c */
esp_err_t start_file_server(const char *base_path);
#ifdef CONFIG_EXAMPLE_MOUNT_SD_CARD
void sdcard_mount(void)
{
    /*sd_card part code*/
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t* card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

#ifndef USE_SPI_MODE
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:
    // slot_config.width = 1;

    // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    // does make a difference some boards, so we do that here.
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
#else
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        ESP_ERROR_CHECK(ret);
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    mount_card = card;
#endif //USE_SPI_MODE
    if(ret != ESP_OK){
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        ESP_ERROR_CHECK(ret);
    }
    sdmmc_card_print_info(stdout, card);

}

static esp_err_t unmount_card(const char* base_path, sdmmc_card_t* card)
{
#ifdef USE_SPI_MODE
    esp_err_t err = esp_vfs_fat_sdcard_unmount(base_path, card);
#else
    esp_err_t err = esp_vfs_fat_sdmmc_unmount();
#endif
    ESP_ERROR_CHECK(err);
#ifdef USE_SPI_MODE
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    err = spi_bus_free(host.slot);
#endif
    ESP_ERROR_CHECK(err);

    return err;
}

#endif
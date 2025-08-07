#include "app_sdcard.h"
#include "FileMeta.h"
#include "config.h"
// #include <Arduino.h>
#include "AudioLib.h"
#include <data_struct.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include "driver/gpio.h"
#include "driver/i2s_common.h"
#include "driver/i2s_std.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdmmc_cmd.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
// SDCard -------------
#define SD_MISO_PIN SD_D0_PIN
#define SD_MOSI_PIN SD_CMD_PIN
#define SD_SCLK_PIN SD_CLK_PIN
#define SD_CS_PIN SD_D3_PIN

#ifndef CONFIG_FATFS_LFN_HEAP
    #error "FATFS LFN support is DISABLED. Please enable it: idf.py menuconfig → Component config → FAT Filesystem support → Use long filenames (LFN)"
#endif

static const char *TAG = "SDCard";


static bool _is_sdcard_mounted = false;

bool is_sdcard_mounted() { return _is_sdcard_mounted; }

void app_sdcard_init() {
  // pinMode(SD_EN_PIN, OUTPUT_OPEN_DRAIN);
  // digitalWrite(SD_EN_PIN, SD_PWD_ON);
  // delay(10);
  gpio_set_level(SD_EN_PIN, SD_PWD_ON); // Enable SD card power

  esp_err_t ret;
  sdmmc_card_t *card;
  const char mount_point[] = MOUNT_POINT;
  ESP_LOGI(TAG, "Initializing SD card");
  // 2. 初始化SD卡
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();

  spi_bus_config_t bus_cfg = {
      .mosi_io_num = (gpio_num_t)SD_MOSI_PIN,
      .miso_io_num = (gpio_num_t)SD_MISO_PIN,
      .sclk_io_num = (gpio_num_t)SD_CLK_PIN,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };

  ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg,
                           SDSPI_DEFAULT_DMA);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize bus.");
    return;
  }

  // This initializes the slot without card detect (CD) and write protect (WP)
  // signals. Modify slot_config.gpio_cd and slot_config.gpio_wp if your board
  // has these signals.
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = (gpio_num_t)SD_CS_PIN;
  slot_config.host_id = (spi_host_device_t)host.slot;

  ESP_LOGI(TAG, "Mounting filesystem");
  // Options for mounting the filesystem.
  // If format_if_mount_failed is set to true, SD card will be partitioned and
  // formatted in case when mounting fails.
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
      .format_if_mount_failed = true,
#else
      .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
      .max_files = 5,
      .allocation_unit_size = 16 * 1024};
  ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config,
                                &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount filesystem. "
                    "If you want the card to be formatted, set the "
                    "CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
    } else {
      ESP_LOGE(TAG,
               "Failed to initialize the card (%s). "
               "Make sure SD card lines have pull-up resistors in place.",
               esp_err_to_name(ret));
#ifdef CONFIG_EXAMPLE_DEBUG_PIN_CONNECTIONS
      check_sd_card_pins(&config, pin_count);
#endif
    }
    return;
  }
  _is_sdcard_mounted = true;

  ESP_LOGI(TAG, "Filesystem mounted");
}


void list_files_with_permissions(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory: %s", dir_path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 构造完整路径

        std::string full_pathstd = std::string(dir_path) + "/" + entry->d_name;
        auto full_path = full_pathstd.c_str();

        // 获取文件信息
        struct stat st;
        if (stat(full_path, &st) == 0) {
            const char *type = S_ISDIR(st.st_mode) ? "DIR " : "FILE";

            // 检查读写权限（对当前进程）
            char perms[4] = "---";
            if (access(full_path, R_OK) == 0) perms[0] = 'R';
            if (access(full_path, W_OK) == 0) perms[1] = 'W';

            ESP_LOGI(TAG, "%s %-20s (%s)", type, entry->d_name, perms);
        } else {
            ESP_LOGW(TAG, "stat failed for %s", full_path);
        }
    }

    closedir(dir);
}

void listFiles(const char *dirPath, BinaryPacker &b, bool hasParent,
               uint16_t page) {
  std::string path = MOUNT_POINT;
  path += dirPath;
  if (!path.empty() && path.back() == '/') {
    path.pop_back();
  }
  DIR *dir = opendir(path.c_str());
  if (!dir) {
    ESP_LOGE(TAG, "Failed to open directory: %s", path.c_str());
    return;
  }
  closedir(dir);

  // list_files_with_permissions(path.c_str());

  ESP_LOGD(TAG, "Scan directory: %s", path.c_str());
  char indexPath[100] = {0};
  snprintf(indexPath, sizeof(indexPath), "%s/audio_index.bin", path.c_str());
  FILE *indexFile = fopen(indexPath, "rb");
  if (!indexFile) {
    ESP_LOGE(TAG, "无法打开索引文件：%s", indexPath);
    return;
  }

  // 计算总记录数和总页数
  if (fseek(indexFile, 0, SEEK_END) != 0) {
    ESP_LOGE(TAG, "Failed to seek to end");
    return;
  }
  long size = ftell(indexFile);
  size_t totalRecords = size / sizeof(FileMeta);
  
  int totalPages = (totalRecords + AUDIO_PAGE_SIZE - 1) / AUDIO_PAGE_SIZE;
  b.writeUint16(totalPages);
  b.writeUint16(totalRecords + (hasParent ? 1 : 0));
  if (hasParent) {
    b.writeUint8(0);
    b.writeString("..");
  }

  // 检查页码有效性
  if (page < 1 || page > totalPages) {
    fclose(indexFile);
    return;
  }

  size_t recordSize = sizeof(FileMeta);
  size_t startPos = (page - 1) * AUDIO_PAGE_SIZE * recordSize;

  if (fseek(indexFile, startPos, SEEK_SET) != 0) {
    ESP_LOGE(TAG, "定位索引文件位置失败");
    fclose(indexFile);
    return;
  }
  FileMeta meta;
  for (int i = 0; i < AUDIO_PAGE_SIZE; i++) {
    if (ftell(indexFile) >= size)
      break;

    if (fread((char *)&meta, recordSize, 1, indexFile) == 1) {
      b.writeUint8(meta.isFile ? 1 : 0);
      b.writeString(meta.fileName);
      if (meta.isFile) {
        b.writeUint32(meta.size);     // 写入文件大小
        b.writeUint32(meta.duration); // 写入时长
      }
    } else {
      break;
    }
  }

  fclose(indexFile);
}

bool file_exists(const char *file) {
  std::string path = MOUNT_POINT;
  path += file;
  const char *fullfile = path.c_str();
  struct stat file_stat;
  return (stat(fullfile, &file_stat) == 0);
}

String getParentPath(String path) {
  int lastSlashIndex = path.lastIndexOf('/'); // 找到最后一个斜杠的位置
  if (lastSlashIndex <= 0) {
    // 如果没有找到斜杠，或者路径本身是根路径，则返回 "/"
    return "/";
  }
  return path.substring(0, lastSlashIndex); // 截取从头到最后一个斜杠之前的部分
}


void writeFile(const char *file, const uint8_t *data, size_t size, const char *mode) {
  std::string path = MOUNT_POINT;
  path += file;
  FILE *f = fopen(path.c_str(), mode);
  if (!f) {
    ESP_LOGE(TAG, "Failed to open file for writing: %s", path.c_str());
    return;
  }
  fwrite(data, 1, size, f);
  fclose(f);
}
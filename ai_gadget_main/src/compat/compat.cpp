
#include "compat.h"
#include "esp_psram.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <esp_timer.h>
#include "esp_private/esp_psram_extram.h"

static volatile bool spiramDetected = false;
static volatile bool spiramFailed = false;

//allows user to bypass SPI RAM test routine
__attribute__((weak)) bool testSPIRAM(void) {
  return esp_psram_extram_test();
}

bool psramInit() {
  if (spiramDetected) {
    return true;
  }
#ifndef CONFIG_SPIRAM_BOOT_INIT
  if (spiramFailed) {
    return false;
  }
#if CONFIG_IDF_TARGET_ESP32
  uint32_t chip_ver = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_PACKAGE);
  uint32_t pkg_ver = chip_ver & 0x7;
  if (pkg_ver == EFUSE_RD_CHIP_VER_PKG_ESP32D2WDQ5 || pkg_ver == EFUSE_RD_CHIP_VER_PKG_ESP32PICOD2) {
    spiramFailed = true;
    ESP_EARLY_LOGW(TAG, "PSRAM not supported!");
    return false;
  }
#elif CONFIG_IDF_TARGET_ESP32S2
  extern void esp_config_data_cache_mode(void);
  esp_config_data_cache_mode();
  Cache_Enable_DCache(0);
#endif
  if (esp_psram_init() != ESP_OK) {
    spiramFailed = true;
    ESP_LOGW(TAG, "PSRAM init failed!");
#if CONFIG_IDF_TARGET_ESP32
    if (pkg_ver != EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4) {
      pinMatrixOutDetach(16, false, false);
      pinMatrixOutDetach(17, false, false);
    }
#endif
    return false;
  }
  //testSPIRAM() allows user to bypass SPI RAM test routine
  if (!testSPIRAM()) {
    spiramFailed = true;
    ESP_LOGE(TAG, "PSRAM test failed!");
    return false;
  }
  //ESP_LOGI(TAG, "PSRAM enabled");
#endif /* CONFIG_SPIRAM_BOOT_INIT */
  spiramDetected = true;
  return true;
}

bool IRAM_ATTR psramFound() {
    return esp_psram_is_initialized();
}

void * IRAM_ATTR ps_malloc(size_t size) {
    if (!esp_psram_is_initialized()) {
        return NULL;
    }
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void IRAM_ATTR *ps_calloc(size_t n, size_t size) {
  if (!spiramDetected) {
    return NULL;
  }
  return heap_caps_calloc(n, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void IRAM_ATTR *ps_realloc(void *ptr, size_t size) {
  if (!spiramDetected) {
    return NULL;
  }
  return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

unsigned long IRAM_ATTR millis() {
  return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

static void reverse(char *begin, char *end) {
  char *is = begin;
  char *ie = end - 1;
  while (is < ie) {
    char tmp = *ie;
    *ie = *is;
    *is = tmp;
    ++is;
    --ie;
  }
}

char *ltoa(long value, char *result, int base) {
  if (base < 2 || base > 16) {
    *result = 0;
    return result;
  }

  char *out = result;
  long quotient = abs(value);

  do {
    const long tmp = quotient / base;
    *out = "0123456789abcdef"[quotient - (tmp * base)];
    ++out;
    quotient = tmp;
  } while (quotient);

  // Apply negative sign
  if (value < 0) {
    *out++ = '-';
  }

  reverse(result, out);
  *out = 0;
  return result;
}

char *lltoa(long long val, char *result, int base) {
  if (base < 2 || base > 16) {
    *result = 0;
    return result;
  }

  char *out = result;
  long long quotient = val > 0 ? val : -val;

  do {
    const long long tmp = quotient / base;
    *out = "0123456789abcdef"[quotient - (tmp * base)];
    ++out;
    quotient = tmp;
  } while (quotient);

  // Apply negative sign
  if (val < 0) {
    *out++ = '-';
  }

  reverse(result, out);
  *out = 0;
  return result;
}
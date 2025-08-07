#pragma once

#include "esp_psram.h"
#include "esp_heap_caps.h"
#include <stdio.h>
#include <ctype.h>


#define boolean bool


bool psramInit();

bool IRAM_ATTR psramFound();

void * IRAM_ATTR ps_malloc(size_t size);
void IRAM_ATTR *ps_calloc(size_t n, size_t size);
void IRAM_ATTR *ps_realloc(void *ptr, size_t size);

unsigned long IRAM_ATTR millis();

// Converts the letter c to lower case, if possible.
inline int toLowerCase(int c) {
  return tolower(c);
}

// Converts the letter c to upper case, if possible.
inline int toUpperCase(int c) {
  return toupper(c);
}

char *ltoa(long value, char *result, int base);
char *lltoa(long long val, char *result, int base);
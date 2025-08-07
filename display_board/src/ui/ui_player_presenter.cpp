#include "ui_player_presenter.h"
#include "ui_player_inc.h"
#include "ui_presenter.h"
#include <deque>
#include <iostream>
#include <string.h>

std::deque<FileMeta> fileList;
std::deque<uint16_t> pageList;
static uint16_t s_total_page = 1;
static uint16_t curr_page = 1;
char *curr_title = nullptr;
static uint32_t track_id = 0;

void player_track_load(uint32_t id) {
  uint32_t old_id = track_id;
  track_id = id;
  curr_title = fileList[id].fileName;
  player_on_track_load(old_id, id);
}

bool player_is_current_playing(const char *title) {
  if (title == nullptr || curr_title == nullptr) {
    return false; // 如果任意一个是 nullptr，则不能匹配
  }
  return strcmp(title, curr_title) == 0;
}

int find_next_audio(bool forward) {
  int index = track_id;
  if (fileList.empty() || index < 0 || index >= fileList.size()) {
    return -1; // 边界检查
  }

  int size = fileList.size();
  int i = index;

  do {
    if (forward) {
      i = (i + 1) % size; // 循环前进
    } else {
      i = (i - 1 + size) % size; // 循环后退
    }

    if (fileList[i].isFile) {
      return i;
    }
  } while (i != index); // 当回到原始索引时停止

  return -1; // 没找到符合条件的项
}

void player_flush_page(uint16_t page, uint16_t total_page) {
  s_total_page = total_page;
  curr_page = page;
}

uint16_t player_data_total_page() {
  return s_total_page;
}

void player_clean_data(uint16_t page) {
  if (page == curr_page) {
    fileList.clear();
    pageList.clear();
    player_notify_data_clear();
    return;
  }
  for (int i = pageList.size() - 1; i >= 0; --i) {
    if (pageList[i] != page) {
      fileList.erase(fileList.begin() + i);
      pageList.erase(pageList.begin() + i);
      player_notify_data_remove(i);
    }
  }
}

void player_add_data(uint16_t page, FileMeta &data) {
  if (page < curr_page) {
    fileList.push_front(data);
    pageList.push_front(page);
    player_notify_data_add_front(data);
  } else {
    fileList.push_back(data);
    pageList.push_back(page);
    player_notify_data_add_back(data);
  }
}

FileMeta *player_get_data(int index) {
  if (index < 0 || index >= static_cast<int>(fileList.size())) {
    return nullptr; // 避免越界访问
  }
  return &fileList[index];
}

FileMeta* player_current_data() {
  return &fileList[track_id];
}

int player_data_size() { return fileList.size(); }

uint16_t player_get_curr_page() { return curr_page; }

uint16_t player_get_total_page() { return s_total_page; }
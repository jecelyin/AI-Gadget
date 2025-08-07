#include "BinaryPacker.h"

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

BinaryPacker::BinaryPacker() {}

// 加载外部数据到缓冲区
void BinaryPacker::load(uint8_t *data, uint16_t length) {
  buffer.clear();                                   // 清空当前缓冲区
  buffer.insert(buffer.end(), data, data + length); // 插入新的数据
  offset = 0;                                       // 重置读取偏移量
}

// 获取当前缓冲区的大小
size_t BinaryPacker::size() const { return buffer.size(); }

// 获取当前的偏移量
size_t BinaryPacker::getOffset() const { return offset; }

const uint8_t *BinaryPacker::data() { return buffer.data(); }

bool BinaryPacker::available() const {
  return buffer.size() > 0 && offset < buffer.size();
}

// 将数据写入缓冲区
void BinaryPacker::writeUint8(uint8_t value) { buffer.push_back(value); }

void BinaryPacker::writeUint16(uint16_t value) {
  buffer.push_back(static_cast<uint8_t>(value & 0xFF));        // 低字节
  buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF)); // 高字节
}

void BinaryPacker::writeUint32(uint32_t value) {
  buffer.push_back(static_cast<uint8_t>(value & 0xFF));         // 低字节
  buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));  // 次低字节
  buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF)); // 次高字节
  buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF)); // 高字节
}

void BinaryPacker::writeFloat(float value) {
  uint8_t *bytes = reinterpret_cast<uint8_t *>(&value);
  buffer.insert(buffer.end(), bytes,
                bytes + sizeof(float)); // 将 4 字节的浮点数插入到缓冲区
}

// Write a C-style string (char*) to the buffer
void BinaryPacker::writeString(const char *value) {
  uint16_t length = strlen(value); // Get the length of the C-string
  writeUint16(length);             // Write the length to the buffer
  buffer.insert(buffer.end(), value,
                value + length); // Write the string's content
}

void BinaryPacker::writeString(const std::string &value) {
  writeUint16(value.length()); // 写入字符串的长度 (uint16_t)
  buffer.insert(buffer.end(), value.begin(), value.end()); // 写入字符串的内容
}

void BinaryPacker::writeString(const String &value) {
  writeUint16(value.length()); // 写入字符串的长度 (uint16_t)
  buffer.insert(buffer.end(), value.begin(), value.end()); // 写入字符串的内容
}

// 从缓冲区读取数据
uint8_t BinaryPacker::readUint8() { return buffer[offset++]; }

uint16_t BinaryPacker::readUint16() {
  uint16_t value = buffer[offset] | (buffer[offset + 1] << 8);
  offset += 2;
  return value;
}

uint32_t BinaryPacker::readUint32() {
  uint32_t value = buffer[offset] | (buffer[offset + 1] << 8) |
                   (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
  offset += 4;
  return value;
}

float BinaryPacker::readFloat() {
  float value;
  uint8_t *bytes = reinterpret_cast<uint8_t *>(&value);
  for (size_t i = 0; i < sizeof(float); i++) {
    bytes[i] = buffer[offset++];
  }
  return value;
}

std::string BinaryPacker::readString() {
  uint16_t length = readUint16(); // 读取字符串长度
  std::string result(buffer.begin() + offset, buffer.begin() + offset + length);
  offset += length;
  return result;
}

String BinaryPacker::readWString() {
  uint16_t length = readUint16(); // 读取字符串长度
  if (length == 0) {
    return String(""); // 如果长度为 0，返回空字符串
  }

  // 利用 String 构造函数一次性构造字符串
  String result((const char *)&buffer[offset], length);

  // 更新偏移量
  offset += length;

  return result;
}

// 清空缓冲区
void BinaryPacker::clear() {
  buffer.clear();
  offset = 0;
}

// 打印当前缓冲区内容（调试用）
void BinaryPacker::printBuffer() const {
  std::cout << "Buffer: ";
  for (uint8_t byte : buffer) {
    std::cout << (int)byte << " ";
  }
  std::cout << std::endl;
}

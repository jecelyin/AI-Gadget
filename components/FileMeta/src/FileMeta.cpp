#include "FileMeta.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>


// 中文数字转换到阿拉伯数字的映射
std::map<std::string, int> chineseToNum = {
    {"一", 1}, {"二", 2}, {"三", 3}, {"四", 4}, {"五", 5},
    {"六", 6}, {"七", 7}, {"八", 8}, {"九", 9}, {"十", 10}};

// 函数：将中文数字转换为阿拉伯数字
int convertChineseToNumber(const std::string &chinese) {
  if (chineseToNum.find(chinese) != chineseToNum.end()) {
    return chineseToNum[chinese];
  }
  return 0;
}

// 函数：提取文件名中的数字部分，并返回数字和剩余字符部分
int extractNumberFromPrefix(const std::string &fileName,
                            std::string &restPart) {
  std::string numberPart;
  size_t i = 0;

  // 逐字符提取数字或中文数字部分
  while (i < fileName.length() &&
         (isdigit(fileName[i]) || isalpha(fileName[i]))) {
    numberPart += fileName[i];
    ++i;
  }

  // 处理中文数字部分（如“一”、“二”等）
  int number = 0;
  if (chineseToNum.find(numberPart) != chineseToNum.end()) {
    number = convertChineseToNumber(numberPart);
  } else {
    // 处理阿拉伯数字
    for (size_t j = 0; j < numberPart.size(); ++j) {
      if (isdigit(numberPart[j])) {
        number = number * 10 + (numberPart[j] - '0');
      }
    }
  }

  // 剩余的部分是文件名的其他字符
  restPart = fileName.substr(i);

  return number;
}

// 自定义比较函数
bool customCompare(const std::string &a, const std::string &b) {
  size_t i = 0;
  std::string restA, restB;

  // 逐字符比较，直到不同
  while (i < a.length() && i < b.length()) {
    if (a[i] == b[i]) {
      ++i;
      continue;
    }

    // 提取前缀数字部分进行比较
    int numA = extractNumberFromPrefix(a.substr(i), restA);
    int numB = extractNumberFromPrefix(b.substr(i), restB);

    // 如果数字不同，按数字大小排序
    if (numA != numB) {
      return numA < numB;
    }

    // 如果数字相同，按剩余部分（字符部分）进行排序
    return restA < restB;
  }

  return a < b; // 如果完全相同，按字典顺序排序
}


// 比较函数，用于排序
bool compareFileMeta(const FileMeta& a, const FileMeta& b) {
  std::string name1, name2;
  name1 = a.fileName;
  name2 = b.fileName;
  return customCompare(name1, name2);
}

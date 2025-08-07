#pragma once
#include <string>
#include <memory>

namespace fs {
class File;

class File {
public:
    enum SeekMode { SeekSet = SEEK_SET, SeekCur = SEEK_CUR, SeekEnd = SEEK_END };

    File(const char* path = nullptr, const char* mode = "r");
    ~File();

    size_t write(uint8_t);
    size_t write(const uint8_t *buf, size_t size);

    int available();
    int read();
    int peek();
    void flush();

    size_t read(uint8_t *buf, size_t size);
    size_t readBytes(char *buffer, size_t length) {
        return read((uint8_t *)buffer, length);
    }

    bool seek(uint32_t pos, SeekMode mode);
    bool seek(uint32_t pos) { return seek(pos, SeekSet); }
    size_t position() const;
    size_t size() const;

    void close();
    operator bool() const { return _fp != nullptr; }

    time_t getLastWrite();
    const char* path() const { return _path.c_str(); }
    const char* name() const;

    bool isDirectory();


private:
    FILE* _fp = nullptr;
    struct dirent* _dirent = nullptr;
    std::string _path;
    std::string _mode;
};

class FS {
public:

    File open(const char* path, const char* mode = "r", bool create = false);
    File open(const std::string& path, const char* mode = "r", bool create = false);

    bool exists(const char* path);
    bool exists(const std::string& path);

    bool remove(const char* path);
    bool remove(const std::string& path);

    bool rename(const char* pathFrom, const char* pathTo);
    bool rename(const std::string& pathFrom, const std::string& pathTo);

    bool mkdir(const char* path);
    bool mkdir(const std::string& path);

    bool rmdir(const char* path);
    bool rmdir(const std::string& path);

};

}  // namespace fs

using fs::File;
using fs::FS;
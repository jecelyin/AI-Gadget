#include "FS.h"
#include <sys/stat.h>
#include <unistd.h>

static std::string mountpoint = "/sdcard";  // 默认挂载点
using namespace fs;


File::File(const char* path, const char* mode) {
    if (path) {
        _fp = fopen(path, mode);
        if (_fp) {
            _path = path;
            _mode = mode;
        }
    }
}

File::~File() {
    close();
}

size_t File::write(uint8_t b) {
    return write(&b, 1);
}

size_t File::write(const uint8_t *buf, size_t size) {
    if (!_fp) return 0;
    return fwrite(buf, 1, size, _fp);
}

int File::available() {
    if (!_fp) return 0;
    long current = ftell(_fp);
    fseek(_fp, 0, SEEK_END);
    long end = ftell(_fp);
    fseek(_fp, current, SEEK_SET);
    return end - current;
}

int File::read() {
    if (!_fp) return -1;
    uint8_t b;
    if (fread(&b, 1, 1, _fp) == 1) return b;
    return -1;
}

size_t File::read(uint8_t *buf, size_t size) {
    if (!_fp) return 0;
    return fread(buf, 1, size, _fp);
}

int File::peek() {
    if (!_fp) return -1;
    int c = fgetc(_fp);
    if (c != EOF) ungetc(c, _fp);
    return c;
}

void File::flush() {
    if (_fp) fflush(_fp);
}

bool File::seek(uint32_t pos, SeekMode mode) {
    if (!_fp) return false;
    return fseek(_fp, pos, mode) == 0;
}

size_t File::position() const {
    if (!_fp) return (size_t)-1;
    return ftell(_fp);
}

size_t File::size() const {
    if (_path.empty()) return 0;
    struct stat st;
    if (stat(_path.c_str(), &st) == 0) return st.st_size;
    return 0;
}

void File::close() {
    if (_fp) {
        fclose(_fp);
        _fp = nullptr;
    }
}

time_t File::getLastWrite() {
    if (_path.empty()) return 0;
    struct stat st;
    if (stat(_path.c_str(), &st) == 0) return st.st_mtime;
    return 0;
}

const char* File::name() const {
    auto slash = _path.find_last_of('/');
    if (slash == std::string::npos) return _path.c_str();
    return _path.c_str() + slash + 1;
}

bool File::isDirectory() {
    struct stat st;
    if (stat(_path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return false;
}


File FS::open(const char* path, const char* mode, bool create) {
    std::string fullPath = mountpoint + path;
    if (!create && access(fullPath.c_str(), F_OK) != 0) {
        return File();  // 文件不存在且不允许创建
    }
    return File(fullPath.c_str(), mode);
}

File FS::open(const std::string& path, const char* mode, bool create) {
    return open(path.c_str(), mode, create);
}

bool FS::exists(const char* path) {
    std::string fullPath = mountpoint + path;
    return access(fullPath.c_str(), F_OK) == 0;
}

bool FS::exists(const std::string& path) {
    return exists(path.c_str());
}

bool FS::remove(const char* path) {
    std::string fullPath = mountpoint + path;
    return ::remove(fullPath.c_str()) == 0;
}

bool FS::remove(const std::string& path) {
    return remove(path.c_str());
}

bool FS::rename(const char* pathFrom, const char* pathTo) {
    std::string fullFrom = mountpoint + pathFrom;
    std::string fullTo = mountpoint + pathTo;
    return ::rename(fullFrom.c_str(), fullTo.c_str()) == 0;
}

bool FS::rename(const std::string& pathFrom, const std::string& pathTo) {
    return rename(pathFrom.c_str(), pathTo.c_str());
}

bool FS::mkdir(const char* path) {
    std::string fullPath = mountpoint + path;
    return ::mkdir(fullPath.c_str(), 0755) == 0;
}

bool FS::mkdir(const std::string& path) {
    return mkdir(path.c_str());
}

bool FS::rmdir(const char* path) {
    std::string fullPath = mountpoint + path;
    return ::rmdir(fullPath.c_str()) == 0;
}

bool FS::rmdir(const std::string& path) {
    return rmdir(path.c_str());
}


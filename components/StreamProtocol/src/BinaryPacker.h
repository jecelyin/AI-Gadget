#pragma once

#include <vector>
#include <string>
#include <WString.h>

class BinaryPacker {
public:
    BinaryPacker();

    void load(uint8_t *data, uint16_t length);
    
    bool available() const;
    size_t size() const;
    size_t getOffset() const;
    const uint8_t* data();
    void clear();

    void writeUint8(uint8_t value);
    void writeUint16(uint16_t value);
    void writeUint32(uint32_t value);
    void writeFloat(float value);
    void writeString(const char *value);
    void writeString(const std::string& value);
    void writeString(const String& value);

    uint8_t readUint8();
    uint16_t readUint16();
    uint32_t readUint32();
    float readFloat();
    std::string readString();
    String readWString();

    void printBuffer() const;

private:
    std::vector<uint8_t> buffer;
    size_t offset = 0;
};


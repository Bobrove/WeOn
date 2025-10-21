#include "Serializer.h"
#include <cstring>

namespace StaticSerializer {

    // ——— Запись данных в буфер ———
    void writeInt(uint8_t* buffer, size_t& offset, size_t capacSity, int value) {
        if (offset + 4 > capacSity) throw std::runtime_error("writeInt: buffer overflow");
        for (int i = 3; i >= 0; --i)
            buffer[offset++] = (value >> (i * 8)) & 0xFF;
    }

    void writeFloat(uint8_t* buffer, size_t& offset, size_t capacity, float value) {
        if (offset + 4 > capacity) throw std::runtime_error("writeFloat: buffer overflow");
        uint32_t bits;
        std::memcpy(&bits, &value, 4);
        writeInt(buffer, offset, capacity, static_cast<int>(bits));
    }

    void writeString(uint8_t* buffer, size_t& offset, size_t capacity, const std::string& str) {
        size_t len = str.size();
        if (offset + 4 + len > capacity) throw std::runtime_error("writeString: buffer overflow");
        writeInt(buffer, offset, capacity, static_cast<int>(len));
        std::memcpy(buffer + offset, str.data(), len);
        offset += len;
    }

    // ——— Чтение данных из буфера ———
    int readInt(const uint8_t* buffer, size_t& offset, size_t capacity) {
        if (offset + 4 > capacity) throw std::runtime_error("readInt: out of range");
        int value = 0;
        for (int i = 0; i < 4; ++i)
            value = (value << 8) | buffer[offset + i];
        offset += 4;
        return value;
    }

    float readFloat(const uint8_t* buffer, size_t& offset, size_t capacity) {
        int bits = readInt(buffer, offset, capacity);
        float value;
        std::memcpy(&value, &bits, 4);
        return value;
    }

    std::string readString(const uint8_t* buffer, size_t& offset, size_t capacity) {
        int len = readInt(buffer, offset, capacity);
        if (len < 0 || offset + len > capacity) throw std::runtime_error("readString: out of range");
        std::string s(reinterpret_cast<const char*>(buffer + offset), len);
        offset += len;
        return s;
    }

} // namespace StaticSerializer

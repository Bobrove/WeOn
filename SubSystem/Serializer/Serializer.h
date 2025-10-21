#pragma once
#include <cstdint>
#include <string>
#include <stdexcept>

namespace StaticSerializer {

    // Функции записи
    void writeInt(uint8_t* buffer, size_t& offset, size_t capacity, int value);
    void writeFloat(uint8_t* buffer, size_t& offset, size_t capacity, float value);
    void writeString(uint8_t* buffer, size_t& offset, size_t capacity, const std::string& str);

    // Функции чтения
    int readInt(const uint8_t* buffer, size_t& offset, size_t capacity);
    float readFloat(const uint8_t* buffer, size_t& offset, size_t capacity);
    std::string readString(const uint8_t* buffer, size_t& offset, size_t capacity);

} // namespace StaticSerializer

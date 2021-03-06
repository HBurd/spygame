#pragma once

#include <cstdint>
#include <cassert>
#include <cstddef>

#define INVALID_INDEX 0xFFFFFFFF

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof (*(x)))

typedef unsigned int uint;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

#define MAKE_ARRAY(name, type, max_size) \
    type name##_internal[max_size]; \
    Array<type> name(name##_internal, 0, max_size)

template <typename T>
struct Array
{
    size_t size = 0;
    size_t max_size = 0;
    T* data = nullptr;

    Array<T>() = default;

    template <size_t N>
    Array<T>(T (*array)[N])
        : size(N), max_size(N), data(*array)
    {}

    Array<T>(T* array, size_t size_, size_t max_size_)
        :size(size_), max_size(max_size_), data(array)
    {}

    Array<T>(T* array, size_t size_)
        :size(size_), max_size(size_), data(array)
    {}

    T* push(T t)
    {
        assert(size < max_size);
        data[size] = t;
        ++size;
        return back();
    }

    T pop()
    {
        assert(size > 0);
        --size;
        T temp = data[size];
        data[size] = {};
        return temp;
    }

    // Overwrites element with the last element of the list
    // (so this does break references)
    T remove(T* element)
    {
        assert(element >= begin() && element < end());

        T element_copy = *element;

        // Note that this still works when element == back()
        *element = *back();
        --size;

        return element_copy;
    }

    void clear()
    {
        size = 0;
    }

    T* back()
    {
        assert(size);
        return &data[size - 1];
    }

    const T* back() const
    {
        assert(size);
        return &data[size - 1];
    }

    T& operator[](size_t idx)
    {
        assert(idx < size);
        return data[idx];
    }

    const T& operator[](size_t idx) const
    {
        assert(idx < size);
        return data[idx];
    }

    const T* begin() const
    {
        return data;
    }

    const T* end() const
    {
        return data + size;
    }

    T* begin()
    {
        return data;
    }

    T* end()
    {
        return data + size;
    }
};

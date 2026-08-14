#ifndef __MINIMALGROWTHARRAY_H
#define __MINIMALGROWTHARRAY_H

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <class T>
class MinimalGrowthArray {
  static_assert(std::is_default_constructible<T>::value, "MinimalGrowthArray<T> requires default-constructible T.");
  static_assert(std::is_copy_constructible<T>::value, "MinimalGrowthArray<T> requires copy-constructible T.");
  static_assert(std::is_copy_assignable<T>::value, "MinimalGrowthArray<T> requires copy-assignable T.");

private:
  std::unique_ptr<T[]> _data;
  std::size_t _size = 0;

public:
  MinimalGrowthArray() = default;

  MinimalGrowthArray(std::size_t size, const T& value)
    : _data(size ? std::make_unique<T[]>(size) : nullptr), _size(size) {
    std::fill(_data.get(), _data.get() + _size, value);
  }

  explicit MinimalGrowthArray(const std::vector<T>& values)
    : _data(values.empty() ? nullptr : std::make_unique<T[]>(values.size())),
      _size(values.size()) {
    std::copy(values.begin(), values.end(), _data.get());
  }

  MinimalGrowthArray(const MinimalGrowthArray& other)
    : _data(other._size ? std::make_unique<T[]>(other._size) : nullptr),
      _size(other._size) {
    std::copy(other._data.get(), other._data.get() + _size, _data.get());
  }

  MinimalGrowthArray& operator=(const MinimalGrowthArray& rhs) {
    if (this == &rhs)
      return *this;

    MinimalGrowthArray tmp(rhs);
    swap(tmp);
    return *this;
  }

  MinimalGrowthArray(MinimalGrowthArray&&) noexcept = default;
  MinimalGrowthArray& operator=(MinimalGrowthArray&&) noexcept = default;

  T& operator[](std::size_t i) {
    return _data[i];
  }

  const T& operator[](std::size_t i) const {
    return _data[i];
  }

  T& at(std::size_t i) {
    if (i >= _size)
      throw std::out_of_range("MinimalGrowthArray::at() index out of range.");
    return _data[i];
  }

  const T& at(std::size_t i) const {
    if (i >= _size)
      throw std::out_of_range("MinimalGrowthArray::at() index out of range.");
    return _data[i];
  }

  T* getArray() {
    return _data.get();
  }

  const T* getArray() const {
    return _data.get();
  }

  std::size_t size() const {
    return _size;
  }

  bool empty() const {
    return _size == 0;
  }

  void clear() noexcept {
    _data.reset();
    _size = 0;
  }

  void set(const std::vector<T>& values) {
    MinimalGrowthArray tmp(values);
    swap(tmp);
  }

  std::vector<T>& get(std::vector<T>& values) const {
    values.resize(_size);
    std::copy(_data.get(), _data.get() + _size, values.begin());
    return values;
  }

  bool exists(const T& value) const {
    return std::find(_data.get(), _data.get() + _size, value) != _data.get() + _size;
  }

  bool operator!=(const MinimalGrowthArray& rhs) const {
    if (_size != rhs._size)
      return true;

    return !std::equal(_data.get(), _data.get() + _size, rhs._data.get());
  }

  void add(const T& value, bool sortAfterAdd = false) {
    std::unique_ptr<T[]> next = std::make_unique<T[]>(_size + 1);

    std::copy(_data.get(), _data.get() + _size, next.get());
    next[_size] = value;

    _data = std::move(next);
    ++_size;

    if (sortAfterAdd)
      std::sort(_data.get(), _data.get() + _size);
  }

  template <typename Functor>
  void sort(Functor functor) {
    std::sort(_data.get(), _data.get() + _size, functor);
  }

  void sort() {
    std::sort(_data.get(), _data.get() + _size);
  }

  void swap(MinimalGrowthArray& other) noexcept {
    using std::swap;
    swap(_data, other._data);
    swap(_size, other._size);
  }
};

#endif

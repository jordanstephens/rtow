
#ifndef PINGPONG_H
#define PINGPONG_H

#include <array>
#include <atomic>
#include <stdexcept>
#include <vector>

template <typename T>
class pingpong {
 public:
  pingpong(size_t size)
      : size(size),
        pages({std::vector<T>(size), std::vector<T>(size)}),
        seals({false, false}) {
    touched[0] = 0;
    touched[1] = 0;
  };

  std::vector<T> get_page(size_t page);
  T get(size_t page, size_t i);
  bool set(size_t page, size_t i, T value);
  void clear_page(size_t page);

 private:
  size_t idx(size_t page, size_t i);

 public:
  size_t size;

 private:
  std::array<std::vector<T>, 2> pages;
  std::array<std::atomic<size_t>, 2> touched;
  std::array<bool, 2> seals;
};

template <typename T>
std::vector<T> pingpong<T>::get_page(size_t page) {
  if (page > 1) throw std::invalid_argument("invalid page number");
  return pages[page];
}

template <typename T>
T pingpong<T>::get(size_t page, size_t i) {
  if (page > 1) throw std::invalid_argument("invalid page number");
  return pages[page][i];
}

template <typename T>
bool pingpong<T>::set(size_t page, size_t i, T value) {
  if (page > 1) throw std::invalid_argument("invalid page number");
  if (seals[page]) throw std::runtime_error("cannot write to sealed page");
  auto count = touched[page] += 1;
  bool seal = count == size;
  if (seal) {
    seals[page] = true;
  }
  pages[page][i] = value;
  return seal;
}

template <typename T>
void pingpong<T>::clear_page(size_t page) {
  if (page > 1) throw std::invalid_argument("invalid page number");
  touched[page] = 0;
  seals[page] = false;
}

#endif

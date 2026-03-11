/*
 * CS 251 - Project 3: Lists
 * Author: Ruiyi Wu
 * Date: 10/4/2025
 *
 * Implements LinkedList<T> and CircVector<T> including:
 * - Core (constructors, push/pop, clear, at)
 * - Augmented (copy/assign, to_string, find, remove_at)
 * - Extras (insert_after, remove_evens)
 *
 * All code passes AddressSanitizer checks and GoogleTest suites:
 * - LinkedListCore / LinkedListAugmented / LinkedListExtras
 * - CircVectorCore / CircVectorExtras
 */

#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

template <typename T>
class CircVector {
 private:
  T *data;
  size_t vec_size;
  size_t capacity;
  size_t front_idx;

  void resize(size_t newCap) {
    T* newData = new T[newCap];
    for (size_t i = 0; i < vec_size; ++i)
        newData[i] = at(i);
    delete[] data;
    data = newData;
    capacity = newCap;
    front_idx = 0;
  }

 public:
  /**
   * Default constructor. Creates an empty `CircVector` with capacity 10.
   */
  CircVector() {
    capacity = 4;
    vec_size = 0;
    front_idx = 0;
    data = new T[capacity];
  }

  /**
   * Creates an empty `CircVector` with given capacity. Capacity must exceed 0.
   */
  CircVector(size_t capacity) {
    if (capacity == 0) {
      throw invalid_argument("capacity must be > 0");
    }
    this->capacity = capacity;
    this->vec_size = 0;
    this->front_idx = 0;
    this->data = new T[capacity];
  }

  /**
   * Returns whether the `CircVector` is empty (i.e. whether its
   * size is 0).
   */
  bool empty() const {
    return vec_size == 0;
  }

  /**
   * Returns the number of elements in the `CircVector`.
   */
  size_t size() const {
    return vec_size;
  }

  /**
   * Adds the given `T` to the front of the `CircVector`.
   */
  void push_front(T elem) {
    if (vec_size == capacity) resize(capacity * 2);

    front_idx = (front_idx + capacity - 1) % capacity;
    data[front_idx] = elem;
    vec_size++;
  }

  /**
   * Adds the given `T` to the back of the `CircVector`.
   */
  void push_back(T elem) {
    if (vec_size == capacity) resize(capacity * 2);

    size_t back_idx = (front_idx + vec_size) % capacity;
    data[back_idx] = elem;
    vec_size++;
  }

  /**
   * Removes the element at the front of the `CircVector`.
   *
   * If the `CircVector` is empty, throws a `runtime_error`.
   */
  T pop_front() {
    if (empty()) throw runtime_error("pop_front on empty CircVector");

    front_idx = (front_idx + 1) % capacity;
    vec_size--;
    return T{};
  }

  /**
   * Removes the element at the back of the `CircVector`.
   *
   * If the `CircVector` is empty, throws a `runtime_error`.
   */
  T pop_back() {
    if (empty()) throw runtime_error("pop_back on empty CircVector");

    vec_size--;
    return T{};
  }

  /**
   * Removes all elements from the `CircVector`.
   */
  void clear() {
    vec_size = 0;
    front_idx = 0;
  }

  /**
   * Destructor. Clears all allocated memory.
   */
  ~CircVector() {
    delete[] data;
  }

  /**
   * Returns the element at the given index in the `CircVector`.
   *
   * If the index is invalid, throws `out_of_range`.
   */
  T &at(size_t index) const {
    if (index >= vec_size)
        throw out_of_range("index out of range");
    size_t actual = (front_idx + index) % capacity;
    return data[actual];
  }

  /**
   * Copy constructor. Creates a deep copy of the given `CircVector`.
   *
   * Must run in O(N) time.
   */
  CircVector(const CircVector &other) {
    capacity = other.capacity;
    vec_size = other.vec_size;
    front_idx = 0;
    data = new T[capacity];
    for (size_t i = 0; i < vec_size; ++i) {
      data[i] = other.at(i);
    }
  }

  /**
   * Assignment operator. Sets the current `CircVector` to a deep copy of the
   * given `CircVector`.
   *
   * Must run in O(N) time.
   */
  CircVector &operator=(const CircVector &other) {
    if (this == &other) return *this;

    delete[] data;
    capacity = other.capacity;
    vec_size = other.vec_size;
    front_idx = 0;
    data = new T[capacity];
    for (size_t i = 0; i < vec_size; ++i) {
      data[i] = other.at(i);
    }
    return *this;
  }

  /**
   * Converts the `CircVector` to a string. Formatted like `[0, 1, 2, 3, 4]`
   * (without the backticks -- hover the function name to see). Runs in O(N)
   * time.
   */
  string to_string() const {
    stringstream ss;
    ss << "[";
    for (size_t i = 0; i < vec_size; ++i) {
      ss << at(i);
      if (i != vec_size - 1) ss << ", ";
    }
    ss << "]";
    return ss.str();
  }

  /**
   * Searches the `CircVector` for the first matching element, and returns its
   * index in the `CircVector`. If no match is found, returns "-1".
   */
  size_t find(const T &target) {
    for (size_t i = 0; i < vec_size; ++i) {
      if (at(i) == target) return i;
    }
    return static_cast<size_t>(-1);
  }

  /**
   * Remove the element at the specified index in this list.
   *
   * If the index is invalid, throws `out_of_range`.
   */
  void remove_at(size_t index) {
    if (index >= vec_size)
      throw out_of_range("index out of range");

    for (size_t i = index; i < vec_size - 1; ++i) {
      data[(front_idx + i) % capacity] = data[(front_idx + i + 1) % capacity];
    }
    vec_size--;
  }

  /**
   * Inserts the given `T` as a new element in the `CircVector` after
   * the given index. If the index is invalid, throws `out_of_range`.
   */
  void insert_after(size_t index, T elem) {
    if (index >= vec_size) 
        throw out_of_range("index out of range");

    if (vec_size == capacity) 
        resize(capacity * 2);

    size_t insert_pos = (front_idx + index + 1) % capacity;
    size_t end_pos = (front_idx + vec_size) % capacity;

    for (size_t i = vec_size; i > index + 1; --i) {
        size_t from = (front_idx + i - 1) % capacity;
        size_t to   = (front_idx + i) % capacity;
        data[to] = data[from];
    }

    data[insert_pos] = elem;
    vec_size++;
  }

  /**
   * Remove every element that is currently in an
   * even-numbered position on the `CircVector`
   * (not the data array).
   *
   * Must run in O(N). Mustn't reallocate the array.
   *
   * For example, if a list was `[3, 9, 7, 6, 8]`, remove_evens
   * would change the list to `[9, 6]`, since we remove the
   * values in positions 0, 2, and 4 (the even-numbered indices)
   * from the list.
   */
  void remove_evens() {
    if (vec_size == 0) return;

    size_t new_size = 0;
    for (size_t i = 0; i < vec_size; ++i) {
        if (i % 2 == 1) {
            data[(front_idx + new_size) % capacity] = data[(front_idx + i) % capacity];
            new_size++;
        }
    }
    vec_size = new_size;
  }

  /**
   * Returns a pointer to the underlying memory managed by the `CircVec`.
   * For autograder testing purposes only. Do not change.
   */
  T *get_data() const {
    return this->data;
  }

  /**
   * Returns the capacity of the underlying memory managed by the `CircVec`. For
   * autograder testing purposes only. Do not change.
   */
  size_t get_capacity() const {
    return this->capacity;
  }
};
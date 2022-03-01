#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include <memory>
#include <thread>
#include <future>
#include <condition_variable>
#include <type_traits>
#include <cassert>

template<typename T>
class vector {
public:
    vector();
    explicit vector(size_t size);
    vector(const vector<T>& other);
    ~vector();
    vector<T>& operator=(const vector<T>& other) noexcept;
    vector(vector<T>&& other);
    vector<T>& operator=(vector<T>&& other) noexcept;
    vector<T> operator+(const vector<T>& other);
    vector<T> operator-(const vector<T>& other);
    vector<T> operator*(const vector<T>& other);
    T& operator[](int i);
    bool empty() const;
    T& at(int i) const; // bounding safe
    size_t getSize() const;
    void reserve(size_t size);
    void resize(size_t size, T elem);
    T& front() const;
    T& back() const;
    void push_back(T&& elem);
    void push_back(const T& elem);
    void pop_back();
    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const vector<U>& ob);
    
private:
    mutable std::mutex mtx;
    std::condition_variable condVar;
    size_t _size;
    size_t _capacity;
    T* _head;
};
template<typename U>
std::ostream& operator<<(std::ostream& os, const vector<U>& ob)
{
    for (int i = 0; i < ob.getSize(); ++i)
    {
        os << ob._head[i] << ' ';
    }
    return os;
}

template<typename T>
vector<T>::vector():_size(0), _capacity(1), _head(new T()){}

template<typename T>
vector<T>::vector(size_t size):_size(size)
{
    size_t _malloc = 1;
    while (_malloc < size) _malloc *= 2;
    _head = new T[_malloc];
    _capacity = _malloc;
}

template<typename T>
vector<T>::vector(const vector<T>& other)
{
    std::unique_lock<std::mutex> _mylock(mtx, std::defer_lock),
                                   _otherlock(other.mtx, std::defer_lock);
    std::lock(_mylock, _otherlock);
    _size = other._size;
    _capacity = other._capacity;
    _head = new T[_capacity];
    for (size_t i = 0; i < _size; ++i) {
        _head[i] = other._head[i];
    }
}

template<typename T>
vector<T>& vector<T>::operator=(const vector<T>& other) noexcept
{
    if (this == &other) return *this;
    std::unique_lock<std::mutex> _mylock(mtx, std::defer_lock),
                                   _otherlock(other.mtx, std::defer_lock);
    std::lock(_mylock, _otherlock);
    if (other._size < _capacity)
    {
        for (size_t i = 0; i < other._size; ++i)
        {
            _head[i] = other._head[i];
        }
        return *this;
    }
    T* temp = new T[other._capacity];
    for (size_t i = 0; i < other._size; ++i)
    {
        temp[i] = other._head[i];
    }
    delete[] _head;
    _size = other._size;
    _capacity = other._capacity;
    _head = temp;
    return *this;
}

template<typename T>
vector<T>::vector(vector<T>&& other)
{
    std::unique_lock<std::mutex> _mylock(mtx, std::defer_lock),
                                   _otherlock(other.mtx, std::defer_lock);
    std::lock(_mylock, _otherlock);
    _size = other._size;
    _capacity = other._capacity;
    _head = other._head;
    other._size = 0;
    other._capacity = 0;
    other._head = nullptr;
}

template<typename T>
vector<T>& vector<T>::operator=(vector<T>&& other) noexcept {
    if (this == &other) return *this;
    std::unique_lock<std::mutex> _mylock(mtx, std::defer_lock),
                                   _otherlock(other.mtx, std::defer_lock);
    std::lock(_mylock, _otherlock);
    delete[] _head;
    _size = other._size;
    _capacity = other._capacity;
    _head = other._head;
    other._size = 0;
    other._capacity = 0;
    other._head = nullptr;
    return *this;
}

template<typename T>
vector<T> vector<T>::operator+(const vector<T>& other) {
    if constexpr (std::is_integral<T>::value) {
        assert(_size == other._size);
        std::unique_lock<std::mutex> _mylock(mtx, std::defer_lock),
                                   _otherlock(other.mtx, std::defer_lock);
        std::lock(_mylock, _otherlock);
        vector<T> temp;
        temp._head = new T[_capacity];
        temp._size = _size;
        temp._capacity = _capacity;
        for (size_t i = 0; i < _size; ++i)
        {
            temp._head[i] = _head[i] + other._head[i];
        }
        return temp;
    } else {
        std::__throw_invalid_argument("bad type\n");
    }
}

template<typename T>
vector<T> vector<T>::operator-(const vector<T>& other) {
    if constexpr (std::is_integral<T>::value) {
        std::unique_lock<std::mutex> _mylock(mtx, std::defer_lock),
                                   _otherlock(other.mtx, std::defer_lock);
        std::lock(_mylock, _otherlock);
        assert(_size == other._size);
        vector<T> temp;
        temp._head = new T[_capacity];
        temp._size = _size;
        temp._capacity = _capacity;
        for (size_t i = 0; i < _size; ++i)
        {
            temp._head[i] = _head[i] - other._head[i];
        }
        return temp;
    } else {
        std::__throw_invalid_argument("bad type\n");
    }
}
template<typename T>
vector<T> vector<T>::operator*(const vector<T>& other) {
    if constexpr (std::is_integral<T>::value) {
        assert(_size == other._size);
        std::unique_lock<std::mutex> _mylock(mtx, std::defer_lock),
                                   _otherlock(other.mtx, std::defer_lock);
        std::lock(_mylock, _otherlock);
        vector<T> temp;
        temp._head = new T[_capacity];
        temp._size = _size;
        temp._capacity = _capacity;
        for (size_t i = 0; i < _size; ++i)
        {
            temp._head[i] = _head[i] * other._head[i];
        }
        return temp;
    } else {
        std::__throw_invalid_argument("bad type\n");
    }
}

template<typename T>
vector<T>::~vector() {
    delete[] _head;
}
template<typename T>
T& vector<T>::operator[](int i) {
    std::lock_guard<std::mutex> lock(mtx);
    return _head[i];
}

template<typename T>
T& vector<T>::at(int i) const {
    if (i < 0 || i > _size - 1) throw std::out_of_range("out of range index");
    std::lock_guard<std::mutex> lock(mtx);
    return _head[i];
}

template<typename T>
bool vector<T>::empty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return _size == 0;
}

template<typename T>
size_t vector<T>::getSize() const {
    std::lock_guard<std::mutex> lock(mtx);
    return _size;
}

template<typename T>
void vector<T>::reserve(size_t size) {
    std::lock_guard<std::mutex> lock(mtx);
    if (size <= _capacity) return;
    size_t _malloc = 1;
    while (_malloc < size) _malloc *= 2;
    T* temp = new T[_malloc];
    for (size_t i = 0; i < _size; ++i) {
        temp[i] = _head[i];
    }
    delete[] _head;
    _capacity = _malloc;
    _head = temp;
}

template<typename T>
void vector<T>::resize(size_t size, T elem) {
    std::lock_guard<std::mutex> lock(mtx);
    reserve(size);
    for (size_t i = _size; i < size; ++i)
    {
        _head[i] = elem;
    }
    _size = size;
}

template<typename T>
T& vector<T>::front() const {
    std::lock_guard<std::mutex> lock(mtx);
    return _head[0];
}

template<typename T>
T& vector<T>::back() const {
    std::lock_guard<std::mutex> lock(mtx);
    return _head[_size - 1];
}

template<typename T>
void vector<T>::push_back(const T& elem){
    std::lock_guard<std::mutex> lock(mtx);
    size_t push = _size;
    if (_size + 1 >= _capacity)
        reserve(_size + 10);
    _head[push] = elem;
    ++_size;
    condVar.notify_one();
}

template<typename T>
void vector<T>::push_back(T&& elem) {
    std::lock_guard<std::mutex> lock(mtx);
    size_t push = _size;
    if (_size + 1 >= _capacity)
        reserve(_size + 10);
    _head[push] = std::move(elem);
    ++_size;
    condVar.notify_one();
}

template<typename T>
void vector<T>::pop_back() {
    std::unique_lock<std::mutex> ulock(mtx);
    condVar.wait(ulock, [this]{
        return !empty();
    });
    --_size;
}

template class vector<std::string>;
template class vector<char>;
#pragma once
#include <iostream>
#include <ranges>
#include <cstdint>
#include "iterator.h"
#include "allocator.h"
#include "reverse_iterator.h"


template<typename T, typename Alloc = std::allocator<T>>
class vector : protected Alloc {
    size_t sz_;
    size_t cap_;
    T* arr_;

    public:

    using value_type = T;
    using allocator_type = std::allocator<T>;
    using size_type = std::size_t;
    using reference = T&;
    // usint pointer = T*; ???
    using iterator = ::base_iterator<false, T>;
    using const_iterator = ::base_iterator<true, T>;
    using reverse_iterator = ::reverse_iterator<iterator>;
    using const_reverse_iterator = ::reverse_iterator<const_iterator>;



    constexpr iterator begin() noexcept {
        return iterator(arr_);
    }

    constexpr iterator end() noexcept {
        return iterator(arr_ + sz_);
    }

    constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    constexpr const_iterator begin() const noexcept {
        return const_iterator(arr_);
    }

    constexpr const_iterator end() const noexcept {
        return const_iterator(arr_ + sz_);
    }

    constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    constexpr const_iterator cbegin() const noexcept {
        return const_iterator(arr_);
    }

    constexpr const_iterator cend() const noexcept {
        return const_iterator(arr_ + sz_);
    }

    constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }


    vector() : sz_(0), cap_(0), arr_(nullptr) {}

    [[nodiscard]] constexpr size_t size() const noexcept {
        return sz_;
    }
    
    [[nodiscard]] constexpr size_t capacity() const noexcept {
        return cap_;
    }
    
    [[nodiscard]] constexpr bool empty() const noexcept {
        return sz_ == 0;
    }
    

    void reserve(size_t new_cap) {
        if (new_cap <= cap_) {
            return;
        }

        T* new_arr = Alloc::allocate(new_cap);

        size_t i = 0;
        try {
            for (; i < sz_; ++i) {
                Alloc::construct(new_arr + i, arr_[i]);
            }
        } catch (...) {
            for (size_t j = 0; j < i; ++j) {
                Alloc::destroy(new_arr + j  );
            }
            Alloc::deallocate(new_arr, new_cap);
            throw;
        }

        for (size_t i = 0; i < sz_; ++i) {
            Alloc::destroy(arr_ + i);
        }
        Alloc::deallocate(arr_, cap_);

        arr_ = new_arr;
        cap_ = new_cap;
        return;
    }

    /*
    
        void reserve(size_t new_cap) {
        if (new_cap <= cap_) {
            return;
        }
        Выделяем помять под новый массив, возникает желание написать так - T* new_arr = new T[new_cap], однако это непреемлемый
        вариант по нескольким причинам:
        1) Противоречит самой цели метода - reserve должен выделить достаточное кол-во памяти, чтобы хранить в векторе
        new_cap объектов типа T, то не создавать эти объекты
        2) У типа T может не быть конструктора по умолчанию
        Поэтому мы будем выделять достаточное кол-во памяти для хнанения new_cap штук объектов типа T, при этом не создавая сами объекты

        // T* new_arr = reinterpret_cast<T*>(new std::byte[new_cap * sizeof(T)]);
        T* new_arr = alloc_.allocate(new_cap);

        Поскольку методы контейнеров должны давать строгую гарантию безопасности отностительно исключений, нужно предусмотреть
        случай, когда конструктор копирования типа T бросит исключение, и обработать данную ситуацию

        size_t i = 0;
        try {

            Копируем элементы в новый массив, и вновь возникает желание написать что-то привычное в духе new_arr[i] = arr_[i],
            однако этот вариант неправильный, поскольку мы вызываем оператор присваивания от двух объектов типа T, но первый из них в 
            реальности не сконструирован, т.е. под new_arr[i] не лежит никакого объекта
            Воспользуемся placement new, чтобы по данному адресу создать объект от заданнх параметров

            for (; i < sz_; ++i) {
                // new(new_arr + i) T(arr_[i]);
                alloc_.construct(new_arr + i, arr_[i]);
            }
        } catch (...) {
            for (size_t j = 0; j < i; ++j) {
                // (new_arr + j)->~T();
                alloc_.destroy(new_arr + j  );
            }
            // delete[] reinterpret_cast<std::byte*>(new_arr);
            alloc_.deallocate(new_arr, new_cap);
            throw;
        }


        Тут кроется ключевой момент сильной гарантии безопасности - если мы не смогли переложить все елементы из arr_ в new_arr
        размера new_cap, то исходный arr_ останется в неизменном состоянии


        for (size_t i = 0; i < sz_; ++i) {
            // (arr_ + i)->~T();
            alloc_.destroy(arr_ + i);
        }
        // delete[] reinterpret_cast<std::byte*>(arr_);
        alloc_.deallocate(arr_, cap_);
        arr_ = new_arr;
        cap_ = new_cap;
        return;
    }
    
    
    */
    
    
    void resize(size_t n, const T& value = T()) {
        if (n < sz_) {
            for (size_t i = n; i < sz_; ++i) {
                (arr_ + i)->~T();
            }
        } else if (n > sz_ && n <= cap_) {
            size_t i = sz_;
            try {
                for (; i < n; ++i) {
                    new(arr_ + i) T(value);
                }
            } catch (...) {
                for (size_t j = sz_; j < i; ++j) {
                    (arr_ + j)->~T();
                }
                throw;
            }
        } else {
            size_t new_cap = cap_ != 0 ? cap_ * 2 : 1;
            if (new_cap < n) {
                new_cap = n;
            }
            T* new_arr = reinterpret_cast<T*>(new std::byte[new_cap * sizeof(T)]);

            size_t i = 0;
            try {
                for (; i < sz_; ++i) {
                    new(new_arr + i) T(arr_[i]);
                }

                for (; i < n; ++i) {
                    new(new_arr + i) T(value);
                }
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    (new_arr + j)->~T();
                }

                delete[] reinterpret_cast<std::byte*>(new_arr);
                throw;
            }

            delete[] reinterpret_cast<std::byte*>(arr_);
            arr_ = new_arr;
            cap_ = new_cap;
        }

        
        sz_ = n;
        return;
    }
    
    void push_back(const T& value) {
        if (sz_ == cap_) {
            size_t new_cap = cap_ != 0 ? cap_ * 2 : 1;
            T* new_arr = reinterpret_cast<T*>(new std::byte[new_cap * sizeof(T)]);

            size_t i = 0;
            try {
                for (; i < sz_; ++i) {
                    new(new_arr + i) T(arr_[i]);
                }
                new(new_arr + sz_) T(value);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    (new_arr + i)->~T();
                }
                delete[] reinterpret_cast<std::byte*>(new_arr);
                throw;
            }

            for (size_t i = 0; i < sz_; ++i) {
                (arr_ + 1)->~T();
            }
            delete[] reinterpret_cast<std::byte*>(arr_);
            arr_ = new_arr;
            cap_ = new_cap;
        } else {
            /*
            Если при создании объекта T(value), будет брошено исключение, то sz_ не увеличится и наш вектор останется 
            в исходном состоянии
            ! Ответственность за ресурсы, которые могли быть выделены при создании T(value) лежит на
            конструкторе этого объекта  
            */
            new(arr_ + sz_) T(value);
        }
        ++sz_;
        return;
    }
    
    void pop_back() {
        /*
        Было бы славно добавить проверку вектора на наличия в нём элементов, но поскольку в stl это не реализовано я также не буду этого делать,
        то есть pop_back, вызванная для пустого ветора приведёт к UB
        */
        --sz_;
        (arr_ + sz_)->~T();
        return;
    }

};
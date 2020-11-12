#ifndef WIZDUMB_REALLOC_VECTOR_HPP
#define WIZDUMB_REALLOC_VECTOR_HPP

#include <cassert>
#include <cstdlib>
#include <algorithm>

constexpr size_t REALLOC_VECTOR_INITIAL_SIZE = 16;
constexpr size_t REALLOC_VECTOR_INCREMENT_SIZE = 32;

template <class T>
class realloc_vector
{
    T      *my_array;
    size_t  my_size;
    size_t  my_allocated_size;

public:
    realloc_vector ()
    {
        my_array = static_cast<T*>(malloc (sizeof(T) * REALLOC_VECTOR_INITIAL_SIZE));
        my_size = 0;
        my_allocated_size = REALLOC_VECTOR_INITIAL_SIZE;
        assert (my_array != nullptr);
    }

    realloc_vector (const realloc_vector &other) :
        my_size { other.my_size },
        my_allocated_size { other.my_allocated_size }
    {
        my_array = static_cast<T*>(malloc (sizeof(T) * other.my_size));
        memcpy (&my_array[0], &other.my_array[0], sizeof(T) * other.my_size);
    }

    realloc_vector (realloc_vector &&other) noexcept :
        my_array { other.my_array },
        my_size { other.my_size },
        my_allocated_size { other.my_allocated_size }
    {
    }

    friend void swap(realloc_vector<T> &first, realloc_vector<T> &second)
    {
        std::swap (first.my_array, second.my_array);
        std::swap (first.my_size, second.my_size);
        std::swap (first.my_allocated_size, second.my_allocated_size);
    }

    realloc_vector<T> & operator=(realloc_vector<T> other)
    {
        swap (*this, other);
        return *this;
    }

    ~realloc_vector ()
    {
        free (my_array);
    }

    [[nodiscard]] size_t size() const
    {
        return my_size;
    }

    const T *begin() const
    {
        return my_array;
    }

    const T *end() const
    {
        return my_array + my_size;
    }

    [[nodiscard]] bool empty() const
    {
        return my_size == 0;
    }

    void push_back(T move)
    {
        if (my_size == my_allocated_size)
        {
            size_t new_size = (my_allocated_size + REALLOC_VECTOR_INCREMENT_SIZE);
            T *new_array = static_cast<T*>(realloc (my_array, new_size * sizeof(T)));
            assert (new_array != nullptr);

            my_array = new_array;
            my_allocated_size = new_size;
        }
        my_array[my_size++] = move;
    }
};


#endif //WIZDUMB_REALLOC_VECTOR_HPP

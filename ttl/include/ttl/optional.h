#pragma once

namespace ttl
{
    template<typename T>
	class optional {
        bool is_valid;
        union storage {
            uint8_t _nothing = 0;
            T data;

            storage() { memset(this, 0, sizeof(storage)); }
            ~storage() {}
        } u;
	public:
        typedef T value_type;

        optional()
            : is_valid(false)
        {}
        optional(T& v)
            : is_valid(true)
        {
            new(&u.data) T(v);
        }
        optional(const T& v)
            : is_valid(true)
        {
            new(&u.data) T(v);
        }
        optional(T&& v)
            : is_valid(true)
        {
            new(&u.data) T(v);
        }
        optional(const optional<T>& other)
            : is_valid(other.is_valid)
        {
            if(is_valid)
                new(&u.data) T(other.u.data);
        }
        ~optional() {
            if(is_valid) {
                (&u.data)->~T();
            }
        }

        optional& operator=(const optional& other) {
            if(is_valid) {
                (&u.data)->~T();
            }
            is_valid = other.is_valid;
            if(is_valid) {
                new(&u.data) T(other.u.data);
            }
            return *this;
        }

        optional& operator=(optional&& other) {
            if(is_valid) {
                (&u.data)->~T();
            }
            is_valid = other.is_valid;
            if(is_valid) {
                new(&u.data) T(std::move(other.u.data));
                (&other.u.data)->~T();
            }
            return *this;
        }

        bool has_value() const noexcept {
            return is_valid;
        }

        operator bool() const noexcept {
            return has_value();
        }

		T& value() noexcept {
            return u.data;
        }

        const T* operator->() const {
            return &u.data;
        }

        T* operator->() {
            return &u.data;
        }

        const T& operator*() const {
            return u.data;
        }

        T& operator*() {
            return u.data;
        }

        template<class... Args>
        T& emplace( Args&&... args ) {
            if(is_valid) {
                (&u.data)->~T();
                is_valid = false;
            }
            new(&u.data) T(std::forward<Args>(args)...);
            is_valid = true;
            return u.data;
        }
	};
}

#pragma once

namespace ttl
{
    template<typename T>
	class optional {
        bool is_valid;
        unsigned char data[sizeof(T)];
	public:
        typedef T value_type;

        optional()
            : is_valid(false)
        {}
        optional(T& v)
            : is_valid(true)
        {
            new(data) T(v);
        }
        optional(const T& v)
            : is_valid(true)
        {
            new(data) T(v);
        }
        optional(T&& v)
            : is_valid(true)
        {
            new(data) T(v);
        }
        ~optional() {
            if(is_valid) {
                reinterpret_cast<T*>(data)->~T();
            }
        }

        bool has_value() const noexcept {
            return is_valid;
        }

        operator bool() const noexcept {
            return has_value();
        }

		T& value() noexcept {
            return *reinterpret_cast<T*>(data);
        }
	};
}

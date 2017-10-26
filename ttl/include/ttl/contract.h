#pragma once
#include <stdexcept>

namespace thalhammer
{
    class contract_failed : public std::logic_error {
    public:
        using std::logic_error::logic_error;
    };

    template<typename T, T vmin, T vmax>
	class contract_value {
        T value;
    public:
        constexpr contract_value(T val)
			: value(val)
		{
            if(val < vmin || val > vmax) {
                throw contract_failed(std::to_string(val) + " is not in range [" + std::to_string(vmin) + ";" + std::to_string(vmax)+"]");
            }
        }

        constexpr contract_value()
			: value(vmin)
		{
        }

        constexpr T min() const {
            return vmin;
        }

        constexpr T max() const {
            return vmax;
        }

        constexpr T val() const {
            return value;
        }

        auto& operator=(T val) {
            if(val < vmin || val > vmax) {
                throw contract_failed(std::to_string(val) + " is not in range [" + std::to_string(vmin) + ";" + std::to_string(vmax)+"]");
            }
            value = val;
            return *this;
        }
	};
	
	template<typename T>
	class contract_not_null {
        T* value;
    public:
        constexpr contract_not_null(T* val)
			: value(val)
		{
            if(val == nullptr)
				throw contract_failed("value is null");
        }

		constexpr T* operator->() const {
            return value;
		}

		T* operator->() {
            return value;
		}
		
		constexpr T& operator*() const {
            return *value;
		}

		T& operator*() {
            return *value;
		}

        auto& operator=(T* val) {
            if(val == nullptr)
				throw contract_failed("value is null");
            value = val;
            return *this;
        }
    };
}
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
        constexpr contract_value(T val) {
            if(val < vmin || val > vmax) {
                throw contract_failed(std::to_string(val) + " is not in range [" + std::to_string(vmin) + ";" + std::to_string(vmax)+"]");
            }
            value = val;
        }

        constexpr contract_value() {
            value = vmin;
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
}
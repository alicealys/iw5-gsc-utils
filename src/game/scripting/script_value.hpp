#pragma once
#include "game/game.hpp"
#include "variable_value.hpp"
#include "vector.hpp"

namespace scripting
{
	class entity;
	class array;
	class function;

	class script_value
	{
	public:
		script_value() = default;
		script_value(const game::VariableValue& value);

		script_value(void* value);

		script_value(int value);
		script_value(unsigned int value);
		script_value(bool value);

		script_value(float value);
		script_value(double value);

		script_value(const char* value);
		script_value(const std::string& value);

		script_value(const entity& value);

		script_value(const vector& value);

		script_value(const array& value);

		script_value(const function& value);

		template <typename T>
		bool is() const;

		template <typename T>
		T as() const
		{
			if (!this->is<T>())
			{
				throw std::runtime_error("Invalid type");
			}

			return get<T>();
		}

		template <typename T, typename I = int>
		T* as_ptr()
		{
			if (!this->is<I>())
			{
				throw std::runtime_error("Invalid type");
			}

			const auto value = this->get<I>();

			if (!value)
			{
				throw std::runtime_error("Null pointer");
			}

			return reinterpret_cast<T*>(value);
		}

		const game::VariableValue& get_raw() const;

		variable_value value_{};
	private:
		template <typename T>
		T get() const;
	};
}

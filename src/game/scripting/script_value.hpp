#pragma once
#include "game/game.hpp"
#include "variable_value.hpp"
#include "vector.hpp"

namespace scripting
{
	class entity;
	class array;
	class function;
	class value_wrap;

	namespace
	{
		std::unordered_map<int, std::string> typenames =
		{
			{0, "undefined"},
			{1, "object"},
			{2, "string"},
			{3, "localized string"},
			{4, "vector"},
			{5, "float"},
			{6, "integer"},
			{7, "codepos"},
			{8, "precodepos"},
			{9, "function"},
			{10, "builtin function"},
			{11, "builtin method"},
			{12, "stack"},
			{13, "animation"},
			{14, "pre animation"},
			{15, "thread"},
			{16, "thread"},
			{17, "thread"},
			{18, "thread"},
			{19, "struct"},
			{20, "removed entity"},
			{21, "entity"},
			{22, "array"},
			{23, "removed thread"},
			{24, "count"},
			{25, "<free>"},
			{26, "thread list"},
			{27, "endon list"},
		};

		std::string get_typename(const game::VariableValue& value)
		{
			if (value.type == game::SCRIPT_OBJECT)
			{
				const auto type = game::scr_VarGlob->objectVariableValue[value.u.uintValue].w.type;
				return typenames[type];
			}
			else
			{
				return typenames[value.type];
			}
		}

		template <typename T, typename A = array>
		std::string get_c_typename()
		{
			auto& info = typeid(T);

			if (info == typeid(std::string))
			{
				return "string";
			}

			if (info == typeid(const char*))
			{
				return "string";
			}

			if (info == typeid(entity))
			{
				return "entity";
			}

			if (info == typeid(array))
			{
				return "array";
			}

			if (info == typeid(function))
			{
				return "function";
			}

			if (info == typeid(vector))
			{
				return "vector";
			}

			return info.name();
		}
	}

	class script_value
	{
	public:
		script_value() = default;
		script_value(const game::VariableValue& value);
		script_value(const value_wrap& value);

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
				const auto type = get_typename(this->get_raw());
				const auto c_type = get_c_typename<T>();
				throw std::runtime_error(std::string("has type '" + type + "' but should be '" + c_type + "'"));
			}

			return get<T>();
		}

		template <typename T, typename I = int>
		T* as_ptr()
		{

			const auto value = this->as<I>();

			if (!value)
			{
				throw std::runtime_error("is null");
			}

			return reinterpret_cast<T*>(value);
		}

		const game::VariableValue& get_raw() const;

		variable_value value_{};
	private:
		template <typename T>
		T get() const;
	};

	class value_wrap
	{
	public:
		value_wrap(const scripting::script_value& value, int argument_index);

		template <typename T>
		T as() const
		{
			try
			{
				return this->value_.as<T>();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(utils::string::va("parameter %d %s", this->argument_index_, e.what()));
			}
		}

		template <typename T, typename I = int>
		T* as_ptr()
		{
			try
			{
				return this->value_.as_ptr<T>();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(utils::string::va("parameter %d %s", this->argument_index_, e.what()));
			}
		}

		template <typename T>
		T is() const
		{
			return this->value_.is<T>();
		}

		const game::VariableValue& get_raw() const
		{
			return this->value_.get_raw();
		}

		int argument_index_{};
		scripting::script_value value_;
	};
}

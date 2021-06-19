#pragma once

namespace gsc
{
	extern std::unordered_map<unsigned, unsigned> replaced_functions;

	class function_args
	{
	public:
		function_args(std::vector<scripting::script_value>);

		unsigned int function_args::size() const;
		std::vector<scripting::script_value> function_args::get_raw() const;
		scripting::script_value get(const int index) const;

		scripting::script_value operator[](const int index) const
		{
			return this->get(index);
		}
	private:
		std::vector<scripting::script_value> values_;
	};

	using builtin_function = void(*)();
	using builtin_method = void(*)(game::scr_entref_t);

	using script_function = std::function<scripting::script_value(function_args)>;
	using script_method = std::function<scripting::script_value(game::scr_entref_t, function_args)>;

	namespace function
	{
		void add(const std::string& name, const script_function& func);
	}

	namespace method
	{
		void add(const std::string& name, const script_method& func);
	}
}
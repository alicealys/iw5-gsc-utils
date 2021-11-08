#pragma once

namespace gsc
{
	enum classid
	{
		entity,
		hudelem,
		pathnode,
		node,
		count
	};

	class function_args
	{
	public:
		function_args(std::vector<scripting::script_value>);

		unsigned int size() const;
		std::vector<scripting::script_value> get_raw() const;
		scripting::value_wrap get(const int index) const;

		scripting::value_wrap operator[](const int index) const
		{
			return this->get(index);
		}
	private:
		std::vector<scripting::script_value> values_;
	};

	using builtin_function = void(*)();
	using builtin_method = void(*)(game::scr_entref_t);

	using script_function = std::function<scripting::script_value(const function_args&)>;
	using script_method = std::function<scripting::script_value(const game::scr_entref_t, const function_args&)>;

	namespace function
	{
		void add(const std::string& name, const script_function& func);
	}

	namespace method
	{
		void add(const std::string& name, const script_method& func);
	}

	namespace field
	{
		void add(classid classnum, const std::string& name,
			const std::function<scripting::script_value(unsigned int entnum)>& getter,
			const std::function<void(unsigned int entnum, const scripting::script_value&)>& setter);
	}
}
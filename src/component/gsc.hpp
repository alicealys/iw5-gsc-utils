#pragma once
#include "game/scripting/array.hpp"
#include "game/scripting/execution.hpp"

namespace gsc
{
	enum class_id_t
	{
		class_entity,
		class_hudelem,
		class_pathnode,
		class_node,
		class_count
	};

	using field_getter_t = std::function<scripting::script_value(unsigned int entnum)>;
	using field_setter_t = std::function<void(unsigned int entnum, const scripting::script_value&)>;

	struct entity_field_t
	{
		std::string name;
		field_getter_t getter;
		field_setter_t setter;
	};

	class function_args
	{
	public:
		function_args();
		function_args(const std::vector<scripting::script_value>&);

		std::uint32_t size() const;
		const std::vector<scripting::script_value>& get_raw() const;
		scripting::value_wrap get(const std::uint32_t index) const;

		scripting::value_wrap operator[](const std::uint32_t index) const
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

	void call_function(const script_function& function, const std::string& name);
	void call_method(const script_method& method, const std::string& name, game::scr_entref_t ent);

	namespace function
	{
		template <typename F>
		void add(const std::string& n, F&& f)
		{
			static auto done = false;
			assert(!done);
			done = true;

			static struct
			{
				std::string name;
				script_function function;
			} ctx{n, f};

			plugin::get()->get_interface()->gsc()->register_function(n, []()
			{
				call_function(ctx.function, ctx.name);
			});
		}
	}

	namespace method
	{
		template <typename F>
		void add(const std::string& n, F&& f)
		{
			static auto done = false;
			assert(!done);
			done = true;

			static struct
			{
				std::string name;
				script_method function;
			} ctx{n, f};

			plugin::get()->get_interface()->gsc()->register_method(n, [](plutonium::sdk::types::entref entref)
			{
				call_method(ctx.function, ctx.name, *reinterpret_cast<game::scr_entref_t*>(&entref));
			});
		}
	}

	namespace field
	{
		void add(const class_id_t classnum, const std::string& name, const field_getter_t getter, const field_setter_t& setter);
	}
}

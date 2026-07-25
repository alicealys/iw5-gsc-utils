#include <stdinc.hpp>
#include "execution.hpp"
#include "stack_isolation.hpp"

namespace scripting
{
	namespace
	{
		game::VariableValue* allocate_argument()
		{
			game::VariableValue* value_ptr = ++game::scr_VmPub->top;
			++game::scr_VmPub->inparamcount;
			return value_ptr;
		}

		script_value get_return_value()
		{
			if (game::scr_VmPub->inparamcount == 0)
			{
				return {};
			}

			game::Scr_ClearOutParams();
			game::scr_VmPub->outparamcount = game::scr_VmPub->inparamcount;
			game::scr_VmPub->inparamcount = 0;

			return script_value(game::scr_VmPub->top[1 - game::scr_VmPub->outparamcount]);
		}
	}

	void push_value(const script_value& value)
	{
		auto* value_ptr = allocate_argument();
		*value_ptr = value.get_raw();

		game::AddRefToValue(value_ptr->type, value_ptr->u);
	}

	script_value exec_ent_thread(const entity& entity, const char* pos,
		const std::vector<script_value>& arguments)
	{
		const auto id = entity.get_entity_id();

		stack_isolation _;
		for (auto i = arguments.rbegin(); i != arguments.rend(); ++i)
		{
			push_value(*i);
		}

		game::AddRefToObject(id);

		const auto local_id = game::AllocThread(id);
		const auto result = game::VM_Execute(local_id, pos, static_cast<unsigned int>(arguments.size()));
		game::RemoveRefToObject(result);

		return get_return_value();
	}

	unsigned int make_array()
	{
		unsigned int index = 0;
		const auto variable = game::AllocVariable(&index);
		variable->w.type = game::SCRIPT_ARRAY;
		variable->u.f.prev = 0;
		variable->u.f.next = 0;

		return index;
	}

	unsigned int make_object()
	{
		unsigned int index = 0;
		const auto variable = game::AllocVariable(&index);
		variable->w.type = game::SCRIPT_STRUCT;
		variable->u.f.prev = 0;

		return index;
	}
}

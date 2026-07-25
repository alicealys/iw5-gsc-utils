#pragma once
#include "game/game.hpp"

/*
 * Swaps scr_VmPub's stack for a private one while native code pushes
 * arguments and runs a GSC thread, then restores it. Without this, running a
 * script thread from inside a builtin clobbers the caller's VM stack.
 */

namespace scripting
{
	class stack_isolation final
	{
	public:
		stack_isolation();
		~stack_isolation();

		stack_isolation(stack_isolation&&) = delete;
		stack_isolation(const stack_isolation&) = delete;
		stack_isolation& operator=(stack_isolation&&) = delete;
		stack_isolation& operator=(const stack_isolation&) = delete;

	private:
		game::VariableValue stack_[512]{};

		game::VariableValue* max_stack_;
		game::VariableValue* top_;
		unsigned int in_param_count_;
		unsigned int out_param_count_;
	};
}

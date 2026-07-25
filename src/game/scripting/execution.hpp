#pragma once
#include "entity.hpp"
#include "script_value.hpp"

/*
 * Two directions:
 *   push_value / make_array / make_object  - native code hands values to the VM
 *   exec_ent_thread                        - native code RUNS a gsc function
 *
 * exec_ent_thread is what makes callbacks (addCommand) possible. The
 * plutonium sdk has no equivalent, so it uses the raw VM symbols directly
 * (AllocThread + VM_Execute); the sdk is only needed for the opposite
 * direction (registering a builtin so gsc can call us).
 */

namespace scripting
{
	void push_value(const script_value& value);

	unsigned int make_array();
	unsigned int make_object();

	script_value exec_ent_thread(const entity& entity, const char* pos,
		const std::vector<script_value>& arguments);
}

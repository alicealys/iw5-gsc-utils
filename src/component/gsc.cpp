#include <stdinc.hpp>
#include <plutonium_sdk.hpp>
#include "gsc.hpp"

/*
 * See gsc.hpp for the overview. Implementation notes:
 *
 * 1. THUNK TABLES: Adapts context-free C callbacks to a registry of distinct
 *    handlers using compile-time-generated free functions.
 *
 * 2. SEH SAFETY NET: Guarded memory access lives in tiny functions with ONLY
 *    POD locals. String/vector/exception building happens in the calling frame.
 *    This prevents server crashes on bad memory access.
 */

namespace gsc
{
	function_args::function_args(std::vector<scripting::script_value> values)
		: values_(std::move(values))
	{
	}

	unsigned int function_args::size() const
	{
		return static_cast<unsigned int>(this->values_.size());
	}

	std::vector<scripting::script_value> function_args::get_raw() const
	{
		return this->values_;
	}

	scripting::value_wrap function_args::get(const int index) const
	{
		if (static_cast<size_t>(index) >= this->values_.size())
		{
			throw std::runtime_error(utils::string::va("parameter %d does not exist", index));
		}

		return {this->values_[index], index};
	}

	namespace
	{
		plutonium::sdk::iinterface* g_interface = nullptr;

		// ── SEH boundary (POD-only frame) ───────────────────────────────────

		enum class call_status
		{
			ok,
			access_violation,
		};

		// `body` is a reference parameter (no local destructor obligation for
		// THIS frame), and the only local is a plain enum - satisfies MSVC's
		// C2712 requirement that a __try-containing function do no object
		// unwinding of its own.
		call_status invoke_guarded(const std::function<void()>& body)
		{
			__try
			{
				body();
				return call_status::ok;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return call_status::access_violation;
			}
		}

		// ── Argument reading (scr_VmPub) ─────────────────────────────────────

		constexpr unsigned int MAX_READABLE_ARGS = 32;

		function_args read_arguments(const std::string& name)
		{
			std::vector<scripting::script_value> args;
			bool crashed = false;
			bool suspicious = false;
			unsigned int observed_count = 0;

			const auto status = invoke_guarded([&]()
			{
				auto* pub = game::scr_VmPub.get();
				observed_count = pub->outparamcount;

				if (pub->outparamcount == 0 || pub->outparamcount > MAX_READABLE_ARGS)
				{
					suspicious = (pub->outparamcount > MAX_READABLE_ARGS);
					return;
				}

				for (unsigned int i = 0; i < pub->outparamcount; i++)
				{
					args.emplace_back(pub->top[-static_cast<int>(i)]);
				}
			});

			if (status == call_status::access_violation)
			{
				log("[iw5-gsc-utils] '" + name + "': ACCESS VIOLATION reading scr_VmPub arguments -- "
					"0x20B4A80 is likely wrong for this build (caught -- server did NOT crash)");
				return function_args({});
			}

			if (suspicious)
			{
				log("[iw5-gsc-utils] '" + name + "': suspicious outparamcount=" + std::to_string(observed_count) +
					" -- scr_VmPub address/timing may be wrong for this build");
			}

			return function_args(std::move(args));
		}

		// ── Return value pushing ─────────────────────────────────────────────

		void return_value(const std::string& name, const scripting::script_value& value)
		{
			const auto raw = value.get_raw(); // no game-memory access, safe outside SEH

			const auto status = invoke_guarded([&]()
			{
				if (game::scr_VmPub->outparamcount)
				{
					game::Scr_ClearOutParams();
				}

				scripting::push_value(raw);
			});

			if (status == call_status::access_violation)
			{
				log("[iw5-gsc-utils] '" + name + "': ACCESS VIOLATION pushing return value -- "
					"Scr_ClearOutParams/AddRefToValue/scr_VmPub may be wrong for this build (caught -- server did NOT crash)");
			}
		}

		// ── Registries ────────────────────────────────────────────────────────

		struct registered_function
		{
			std::string name;
			script_function func;
		};

		struct registered_method
		{
			std::string name;
			script_method func;
		};

		std::vector<registered_function> g_functions;
		std::vector<registered_method> g_methods;

		void invoke_function(size_t index)
		{
			const auto& entry = g_functions[index];

			const auto args = read_arguments(entry.name);

			scripting::script_value result;
			bool threw = false;
			std::string error_message;

			const auto status = invoke_guarded([&]()
			{
				try
				{
					result = entry.func(args);
				}
				catch (const std::exception& e)
				{
					threw = true;
					error_message = e.what();
				}
			});

			if (status == call_status::access_violation)
			{
				log("[iw5-gsc-utils] '" + entry.name + "': ACCESS VIOLATION during execution (caught -- server did NOT crash)");
				return;
			}

			if (threw)
			{
				log("[iw5-gsc-utils] '" + entry.name + "': " + error_message);
				return;
			}

			return_value(entry.name, result);
		}

		void invoke_method(size_t index, plutonium::sdk::types::entref entref)
		{
			const auto& entry = g_methods[index];

			const game::scr_entref_t ent{entref.entnum, entref.classnum};
			const auto args = read_arguments(entry.name);

			scripting::script_value result;
			bool threw = false;
			std::string error_message;

			const auto status = invoke_guarded([&]()
			{
				try
				{
					result = entry.func(ent, args);
				}
				catch (const std::exception& e)
				{
					threw = true;
					error_message = e.what();
				}
			});

			if (status == call_status::access_violation)
			{
				log("[iw5-gsc-utils] '" + entry.name + "': ACCESS VIOLATION during execution (caught -- server did NOT crash)");
				return;
			}

			if (threw)
			{
				log("[iw5-gsc-utils] '" + entry.name + "': " + error_message);
				return;
			}

			return_value(entry.name, result);
		}

		// ── Thunk tables ──────────────────────────────────────────────────────
		// plutonium-sdk's callbacks are context-free, so we generate one real
		// function per registration slot at compile time.

		constexpr size_t MAX_FUNCTIONS = 256;
		constexpr size_t MAX_METHODS = 256;

		template <size_t I>
		void PLUTONIUM_CALLBACK function_thunk()
		{
			invoke_function(I);
		}

		template <size_t I>
		void PLUTONIUM_CALLBACK method_thunk(plutonium::sdk::types::entref entref)
		{
			invoke_method(I, entref);
		}

		template <size_t... Is>
		constexpr auto make_function_thunks(std::index_sequence<Is...>)
		{
			return std::array<plutonium::sdk::interfaces::gsc::function_callback, sizeof...(Is)>{ &function_thunk<Is>... };
		}

		template <size_t... Is>
		constexpr auto make_method_thunks(std::index_sequence<Is...>)
		{
			return std::array<plutonium::sdk::interfaces::gsc::method_callback, sizeof...(Is)>{ &method_thunk<Is>... };
		}

		const auto function_thunks = make_function_thunks(std::make_index_sequence<MAX_FUNCTIONS>{});
		const auto method_thunks = make_method_thunks(std::make_index_sequence<MAX_METHODS>{});
	}

	void init(plutonium::sdk::iinterface* interface_ptr)
	{
		g_interface = interface_ptr;
	}

	void log(const std::string& msg)
	{
		// Forwards to the internal helper so other components can report
		// errors through the same SDK logger.
		if (g_interface)
		{
			g_interface->logging()->info(msg.c_str());
		}
	}

	namespace function
	{
		void add(const std::string& name, const script_function& func)
		{
			if (g_functions.size() >= MAX_FUNCTIONS)
			{
				log("[iw5-gsc-utils] cannot register '" + name + "': MAX_FUNCTIONS (" +
					std::to_string(MAX_FUNCTIONS) + ") reached");
				return;
			}

			const auto index = g_functions.size();
			g_functions.push_back({name, func});

			g_interface->gsc()->register_function(name, function_thunks[index]);
		}
	}

	namespace method
	{
		void add(const std::string& name, const script_method& func)
		{
			if (g_methods.size() >= MAX_METHODS)
			{
				log("[iw5-gsc-utils] cannot register method '" + name + "': MAX_METHODS (" +
					std::to_string(MAX_METHODS) + ") reached");
				return;
			}

			const auto index = g_methods.size();
			g_methods.push_back({name, func});

			g_interface->gsc()->register_method(name, method_thunks[index]);
		}
	}
}

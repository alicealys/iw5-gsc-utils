#pragma once
#include "game/game.hpp"
#include "script_value.hpp"

namespace scripting
{
	class object final
	{
	public:
		object();
		object(const unsigned int);

		object(const object& other);
		object(object&& other) noexcept;

		~object();

		object& operator=(const object& other);
		object& operator=(object&& other) noexcept;

		unsigned int size() const;
		unsigned int get_entity_id() const;

		entity get_raw() const;

	private:
		void add() const;
		void release() const;

		unsigned int id_;
	};
}

#pragma once
#include "game/game.hpp"
#include "script_value.hpp"

namespace scripting
{
	class entity final
	{
	public:
		entity();
		entity(unsigned int entity_id);

		entity(const entity& other);
		entity(entity&& other) noexcept;

		~entity();

		entity& operator=(const entity& other);
		entity& operator=(entity&& other) noexcept;

		unsigned int get_entity_id() const;
		game::scr_entref_t get_entity_reference() const;

		bool operator ==(const entity& other) const noexcept;
		bool operator !=(const entity& other) const noexcept;

	private:
		unsigned int entity_id_;

		void add() const;
		void release() const;
	};
}

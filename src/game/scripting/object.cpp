#include <stdinc.hpp>
#include "object.hpp"
#include "script_value.hpp"
#include "execution.hpp"

namespace scripting
{
	object::object(const unsigned int id)
		: id_(id)
	{
		this->add();
	}

	object::object(const object& other)
		: object(other.id_)
	{
	}

	object::object(object&& other) noexcept
	{
		this->id_ = other.id_;
		other.id_ = 0;
	}

	object::object()
	{
		this->id_ = make_object();
	}

	object::~object()
	{
		this->release();
	}

	object& object::operator=(const object& other)
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			this->add();
		}

		return *this;
	}

	object& object::operator=(object&& other) noexcept
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			other.id_ = 0;
		}

		return *this;
	}

	void object::add() const
	{
		if (this->id_)
		{
			game::AddRefToValue(game::SCRIPT_OBJECT, {static_cast<int>(this->id_)});
		}
	}

	void object::release() const
	{
		if (this->id_)
		{
			game::RemoveRefToValue(game::SCRIPT_OBJECT, {static_cast<int>(this->id_)});
		}
	}

	unsigned int object::size() const
	{
		return game::Scr_GetSelf(this->id_);
	}

	unsigned int object::get_entity_id() const
	{
		return this->id_;
	}

	entity object::get_raw() const
	{
		return entity(this->id_);
	}
}

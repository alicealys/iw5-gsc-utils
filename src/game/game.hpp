#pragma once

namespace game
{
	template <typename T>
	class symbol
	{
	public:
		symbol(const size_t dedi)
			: dedi_(reinterpret_cast<T*>(dedi))
		{
		}

		T* get() const
		{
			return dedi_;
		}

		operator T* () const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}

	private:
		T* dedi_;
	};
}

#include "symbols.hpp"
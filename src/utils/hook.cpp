#include <stdinc.hpp>

// iw6x-client / open-iw5

namespace utils::hook
{
	namespace
	{
		[[maybe_unused]] class _
		{
		public:
			_()
			{
				if (MH_Initialize() != MH_OK)
				{
					throw std::runtime_error("Failed to initialize MinHook");
				}
			}

			~_()
			{
				MH_Uninitialize();
			}
		} __;
	}

	detour::detour(const size_t place, void* target) : detour(reinterpret_cast<void*>(place), target)
	{
	}

	detour::detour(void* place, void* target)
	{
		this->create(place, target);
	}

	detour::~detour()
	{
		this->clear();
	}

	void detour::enable() const
	{
		MH_EnableHook(this->place_);
	}

	void detour::disable() const
	{
		MH_DisableHook(this->place_);
	}

	void detour::create(void* place, void* target)
	{
		this->clear();
		this->place_ = place;

		if (MH_CreateHook(this->place_, target, &this->original_) != MH_OK)
		{
			throw std::runtime_error(string::va("Unable to create hook at location: %p", this->place_));
		}

		this->enable();
	}

	void detour::create(const size_t place, void* target)
	{
		this->create(reinterpret_cast<void*>(place), target);
	}

	void detour::clear()
	{
		if (this->place_)
		{
			MH_RemoveHook(this->place_);
		}

		this->place_ = nullptr;
		this->original_ = nullptr;
	}

	void* detour::get_original() const
	{
		return this->original_;
	}

	void set(std::uintptr_t address, void* buffer, size_t size)
	{
		DWORD oldProtect = 0;

		auto* place = reinterpret_cast<void*>(address);
		VirtualProtect(place, size, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(place, buffer, size);
		VirtualProtect(place, size, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), place, size);
	}

	void jump(std::uintptr_t address, void* destination)
	{
		if (!address) return;

		std::uint8_t bytes[5];
		bytes[0] = 0xE9;
		*reinterpret_cast<std::uint32_t*>(bytes + 1) = CalculateRelativeJMPAddress(address, destination);

		set(address, bytes, 5);
	}
}
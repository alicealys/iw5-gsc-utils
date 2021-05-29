#pragma once

namespace scheduler
{
	void schedule(const std::function<bool()>& callback, std::chrono::milliseconds delay = 0ms);
	void loop(const std::function<void()>& callback, std::chrono::milliseconds delay = 0ms);
	void once(const std::function<void()>& callback, std::chrono::milliseconds delay = 0ms);

	void init();
}

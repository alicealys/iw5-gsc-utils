#pragma once

namespace scripting
{
	void on_shutdown(const std::function<void()>& callback);
}

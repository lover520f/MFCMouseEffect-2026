#pragma once

#include <thread>
#include <atomic>
#include <functional>
#include <string>

namespace mousefx
{
	class IpcController
	{
	public:
		using CommandCallback = std::function<void(const std::string&)>;

		IpcController();
		~IpcController();

		// Start the listening thread.
		void Start(CommandCallback callback);

		// Stop the listening thread (best effort).
		void Stop();

	private:
		void ListenerLoop();

		std::thread worker_;
		std::atomic<bool> running_{ false };
		CommandCallback callback_;
	};
}

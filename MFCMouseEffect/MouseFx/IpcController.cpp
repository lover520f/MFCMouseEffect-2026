#include "pch.h"
#include "IpcController.h"
#include <iostream>
#include <string>

namespace mousefx
{
	IpcController::IpcController() = default;

	IpcController::~IpcController()
	{
		Stop();
	}

	void IpcController::Start(CommandCallback callback)
	{
		if (running_) return;

		callback_ = std::move(callback);
		running_ = true;
		worker_ = std::thread(&IpcController::ListenerLoop, this);
	}

	void IpcController::Stop()
	{
		running_ = false;
		// Detach immediately - can't join because std::getline is blocking.
		// The thread will die when the process exits or stdin closes.
		if (worker_.joinable())
		{
			worker_.detach();
		}
	}

	void IpcController::ListenerLoop()
	{
		// This is the core logic you requested to see.
		// It simply reads from std::cin line by line.
		std::string line;
		while (running_ && std::getline(std::cin, line))
		{
			if (line.empty()) continue;

			if (callback_)
			{
				callback_(line);
			}
		}
		
		// If cin closes (EOF), we also stop.
		running_ = false;
	}
}

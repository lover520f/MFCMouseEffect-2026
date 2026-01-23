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
		if (!running_) return;

		running_ = false;
		// detach is often cleaner for blocking I/O threads that we can't easily cancel
		// without closing the stdin handle (which might affect the whole process).
		// given the app is exiting, detach allows the thread to die with the process.
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

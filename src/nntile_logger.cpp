#include <starpu.h>
#include <starpu_profiling.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <sys/socket.h>
#include "websocket_client.hpp"

std::thread logger_thread;
std::atomic<bool> logger_running(true);

extern "C" void nntile_logger_thread()
{
	int workerid;
	int worker_cnt = starpu_worker_get_count();
	int is_initialized = starpu_is_initialized();
	std::cout << "WORKER COUNT: " << worker_cnt << std::endl;
	std::cout << "IS initialized : " << is_initialized << std::endl;

	while (logger_running)
	{
		for (workerid = 0; workerid < worker_cnt; workerid++)
		{
			struct starpu_profiling_worker_info info;
			int ret = starpu_profiling_worker_get_info(workerid, &info);
			if (ret != 0)
				continue;
			char name[64];
			starpu_worker_get_name(workerid, name, sizeof(name));
			double total_time = starpu_timing_timespec_to_us(&info.total_time) / 1000.;
			double flops = 0.0;
			if (info.flops)
				flops = info.flops;
			char message[256];
			snprintf(message, sizeof(message), "{\"name\": \"%s\", \"total_time\": \"%.2lf\", \"flops\": \"%.2lf\"}\n", name, total_time, flops);
			if (send(client_socket, message, strlen(message), 0) != (ssize_t)strlen(message))
			{
				perror("send");
			}
		}
		usleep(500000);
	}
}

extern "C" void nntile_logger_init()
{
	logger_running = true;
	websocket_connect();
	logger_thread = std::thread(nntile_logger_thread);
}

extern "C" void nntile_logger_shutdown()
{
	logger_running = false;
	if (logger_thread.joinable())
	{
		logger_thread.join();
	}
	websocket_disconnect();
}

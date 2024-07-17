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
    // At first get worker count, bus count and check if starpu is initialized
    int workerid;
    int busid;
    int worker_cnt = starpu_worker_get_count();
    int bus_cnt = starpu_bus_get_count();
    int is_initialized = starpu_is_initialized();
    std::cout << "WORKER COUNT: " << worker_cnt << std::endl;
    std::cout << "BUS COUNT: " << bus_cnt << std::endl;
    std::cout << "IS initialized : " << is_initialized << std::endl;

    // Infinite loop until NNTile exits this thread
    while (logger_running)
    {
        std::stringstream ss;
        ss << "{";

        // Loop through all workers to get their activities
        ss << "\"workers\":[";
        bool first_worker = true;
        for (workerid = 0; workerid < worker_cnt; workerid++)
        {
            // Profiling info is read from StarPU
            struct starpu_profiling_worker_info info;
            int ret = starpu_profiling_worker_get_info(workerid, &info);
            if (ret != 0)
                continue;

            if (!first_worker)
                ss << ",";
            first_worker = false;
            char name[64];
            starpu_worker_get_name(workerid, name, sizeof(name));
            double total_time = starpu_timing_timespec_to_us(&info.total_time) * 1e-6;
            double flops = info.flops ? info.flops : 0.0;

            // Create JSON object for the worker
            ss << "{";
            ss << "\"name\":\"" << name << "\",";
            ss << "\"total_time\":" << total_time << ",";
            ss << "\"flops\":" << flops;
            ss << "}";
        }
        ss << "],";

        // Loop through all buses to get their activities
        ss << "\"buses\":[";
        bool first_bus = true;
        for (busid = 0; busid < bus_cnt; busid++)
        {
            int src, dst;
            char src_name[128], dst_name[128];

            // Profiling info is read from StarPU
            struct starpu_profiling_bus_info info;
            int ret = starpu_bus_get_profiling_info(busid, &info);
            if (ret != 0)
                continue;

            if (!first_bus)
                ss << ",";
            first_bus = false;

            // Read the profiling information for the bus
            double total_bus_time = starpu_timing_timespec_to_us(&info.total_time) * 1e-6;
            uint64_t transferred_bytes = info.transferred_bytes;
            src = starpu_bus_get_src(busid);
            dst = starpu_bus_get_dst(busid);
            starpu_memory_node_get_name(src, src_name, sizeof(src_name));
            starpu_memory_node_get_name(dst, dst_name, sizeof(dst_name));

            // Create JSON object for the bus
            ss << "{";
            ss << "\"total_bus_time\":" << total_bus_time << ",";
            ss << "\"transferred_bytes\":" << transferred_bytes << ",";
            ss << "\"src_name\":\"" << src_name << "\",";
            ss << "\"dst_name\":\"" << dst_name << "\"";
            ss << "}";
        }
        ss << "]";

        ss << "}";

        // Serialize JSON object to string
        std::string message = ss.str() + "\n";

        // Send the message
        if (send(client_socket, message.c_str(), message.length(), 0) != (ssize_t)message.length())
        {
            perror("send");
        }

        // Wait for 0.5 seconds until next time we read activities
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
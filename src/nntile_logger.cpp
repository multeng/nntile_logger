#include <iostream>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <sys/socket.h>
#include "websocket_client.hpp"
#include <starpu.h>

std::thread logger_thread;
std::atomic<bool> logger_running(true);
static WebSocketClient ws_client;

extern "C"
{
    void nntile_logger_thread()
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
                ws_client.send(message);
            }
            usleep(500000);
        }
    }

    void nntile_logger_init()
    {
        ws_client.connect("ws://localhost:5001");
        logger_running = true;
        logger_thread = std::thread(nntile_logger_thread);
    }

    void nntile_logger_shutdown()
    {
        std::cout << "LOGGER SHUTDOWN" << std::endl;
        logger_running = false;
        if (logger_thread.joinable())
        {
            logger_thread.join();
        }
        ws_client.close();
    }
}

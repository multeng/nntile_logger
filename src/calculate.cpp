#include <iostream>
#include <random>
#include <unistd.h>
#include <starpu.h>
#include "nntile_logger.hpp"


#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5001

void acquire_callback(void *arg)
{
    starpu_data_handle_t *sum_handle = (starpu_data_handle_t *)arg;
    float *sum_ptr = (float *)starpu_data_get_local_ptr(*sum_handle);

    char message[256];
    snprintf(message, sizeof(message), "{\"sum\": \"%f\"}\n", *sum_ptr);
    starpu_data_release(*sum_handle);
    std::cout << "DATA : " << message << std::endl;
}


void rand_time_cpu(void *buffers[], void *cl_arg)
{
  struct starpu_vector_interface *sum_iface = (struct starpu_vector_interface *)buffers[0];
  float *sum = (float *)STARPU_VECTOR_GET_PTR(sum_iface);

  float from, to;

  starpu_codelet_unpack_args(cl_arg, &from, &to);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(from, to);
  auto val = dis(gen);
  usleep(val * 1000000);
  *sum = *sum + val;
  std::cout << "val = " << val << std::endl;
}

struct starpu_perfmodel rand_time_pm = {
    .type = STARPU_HISTORY_BASED,
    .symbol = "rand_time",
};

struct starpu_codelet rand_time_cl = {
    .cpu_funcs = {rand_time_cpu},
    .nbuffers = 1,
    .modes = {STARPU_RW},
    .model = &rand_time_pm,
};

int main()
{
  int ret = starpu_init(NULL);
  STARPU_CHECK_RETURN_VALUE(ret, "starpu_init");
  nntile_logger_init();
  float sum[1] = {0};
  starpu_data_handle_t sum_handle;
  starpu_vector_data_register(&sum_handle, STARPU_MAIN_RAM, (uintptr_t)sum, 1, sizeof(sum[0]));
  float from = 0.1, to = 0.3;
  int N = 100;
  for (int i = 0; i < N; i++)
  {
    ret = starpu_task_insert(&rand_time_cl, STARPU_VALUE, &from, sizeof(from),
                             STARPU_VALUE, &to, sizeof(to),
                             STARPU_RW, sum_handle,
                             STARPU_FLOPS, (double)150.111,
                             0);
    STARPU_CHECK_RETURN_VALUE(ret, "starpu_task_insert");
    // starpu_data_handle_t *sum_handle_copy = new starpu_data_handle_t;
    // *sum_handle_copy = sum_handle;
    // starpu_data_acquire_cb(sum_handle, STARPU_R, acquire_callback, sum_handle_copy);
  }
  starpu_task_wait_for_all();
  starpu_data_unregister(sum_handle);
  // std::cout << "N = " << N << std::endl;
  // std::cout << "from = " << from << std::endl;
  // std::cout << "to = " << to << std::endl;
  // std::cout << "sum = " << sum[0] << std::endl;

  nntile_logger_shutdown();
  starpu_shutdown();
  return 0;
}

#include <iostream>
#include <random>
#include <unistd.h>
#include <starpu.h>
#include "starpu_logger.h"

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

struct starpu_codelet rand_time_cl = {
    .cpu_funcs = {rand_time_cpu},
    .nbuffers = 1,
    .modes = {STARPU_RW},
};

int main()
{
  int ret = starpu_init(NULL);
  STARPU_CHECK_RETURN_VALUE(ret, "starpu_init");
  starpu_logger_init();
  float sum[1] = {0};
  // sum[0] = 0;
  starpu_data_handle_t sum_handle;
  starpu_vector_data_register(&sum_handle, STARPU_MAIN_RAM, (uintptr_t)sum, 1, sizeof(sum[0]));
  float from = 0.1, to = 0.3;
  int N = 100;
  for (int i = 0; i < N; i++)
  {
    ret = starpu_task_insert(&rand_time_cl, STARPU_VALUE, &from, sizeof(from),
                             STARPU_VALUE, &to, sizeof(to),
                             STARPU_RW, sum_handle,
                             STARPU_FLOPS, (double)2.111,
                             0);
    STARPU_CHECK_RETURN_VALUE(ret, "starpu_task_insert");
  }
  starpu_task_wait_for_all();
  std::cout << "N = " << N << std::endl;
  std::cout << "from = " << from << std::endl;
  std::cout << "to = " << to << std::endl;
  std::cout << "sum = " << sum[0] << std::endl;
  starpu_data_unregister(sum_handle);
  starpu_logger_shutdown();
  starpu_shutdown();
  return 0;
}
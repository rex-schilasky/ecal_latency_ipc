#include <ecal/ecal.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <numeric>
#include <vector>

// warmup runs not to measure
const int warmups(10);

// helper
long long get_microseconds();
void evaluate(std::vector<long long>& lat_arr_, size_t rec_size_, size_t warmups_);

// data structure for later evaluation
struct SCallbackPar
{
  SCallbackPar() { latency_array.reserve(100000); };
  std::mutex             mtx;
  std::vector<long long> latency_array;
  size_t                 rec_size = 0;
  size_t                 msg_num  = 0;
};

// message receive callback
void on_receive(const struct eCAL::SReceiveCallbackData* data_, SCallbackPar* par_)
{
  // get receive time stamp
  auto rec_time = get_microseconds();

  // update latency, size and msg number
  std::lock_guard<std::mutex> lock(par_->mtx);
  par_->latency_array.push_back(rec_time - data_->time);
  par_->rec_size = data_->size;
  par_->msg_num++;
}

// single test run
void do_run()
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "latency_rec");

  // subscriber
  eCAL::CSubscriber sub("ping");

  // apply subscriber callback function
  SCallbackPar cb_par;
  auto callback = std::bind(on_receive, std::placeholders::_2, &cb_par);
  sub.AddReceiveCallback(callback);

  size_t msg_last(0);
  while (eCAL::Ok())
  {
    // check once a second if we still receive new messages
    // if not, we stop and evaluate this run
    {
      std::lock_guard<std::mutex> lock(cb_par.mtx);
      if ((cb_par.msg_num > 0) && (msg_last == cb_par.msg_num)) break;
      else msg_last = cb_par.msg_num;
    }
    eCAL::Process::SleepMS(1000);
  }

  // detach callback
  sub.RemReceiveCallback();

  // evaluate all
  evaluate(cb_par.latency_array, cb_par.rec_size, warmups);

  // finalize eCAL API
  eCAL::Finalize();
}

int main(int /*argc*/, char** /*argv*/)
{
  // run tests
  do { do_run(); } while (eCAL::Ok());

  return(0);
}

// time getter
long long get_microseconds()
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  return(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());
}

// evaluation
void evaluate(std::vector<long long>& lat_arr_, size_t rec_size_, size_t warmups_)
{
  // remove warmup runs
  if (lat_arr_.size() >= warmups_)
  {
    lat_arr_.erase(lat_arr_.begin(), lat_arr_.begin() + warmups_);
  }

  // evaluate all
  long long sum_msg = lat_arr_.size();
  std::cout << "--------------------------------------------" << std::endl;
  std::cout << "Messages received             : " << sum_msg  << std::endl;
  if (sum_msg > warmups)
  {
    long long sum_time = std::accumulate(lat_arr_.begin(), lat_arr_.end(), 0LL);
    long long avg_time = sum_time / sum_msg;
    auto      min_it = std::min_element(lat_arr_.begin(), lat_arr_.end());
    auto      max_it = std::max_element(lat_arr_.begin(), lat_arr_.end());
    size_t    min_pos = min_it - lat_arr_.begin();
    size_t    max_pos = max_it - lat_arr_.begin();
    long long min_time = *min_it;
    long long max_time = *max_it;
    std::cout << "Message size received         : " << rec_size_ / 1024 << " kB" << std::endl;
    std::cout << "Message average latency       : " << avg_time << " us" << std::endl;
    std::cout << "Message min latency           : " << min_time << " us @ " << min_pos << std::endl;
    std::cout << "Message max latency           : " << max_time << " us @ " << max_pos << std::endl;
    std::cout << "Throughput                    : " << static_cast<int>(((rec_size_ * sum_msg) / 1024.0) / (sum_time / 1000.0 / 1000.0)) << " kB/s" << std::endl;
    std::cout << "                              : " << static_cast<int>(((rec_size_ * sum_msg) / 1024.0 / 1024.0) / (sum_time / 1000.0 / 1000.0)) << " MB/s" << std::endl;
    std::cout << "                              : " << static_cast<int>(sum_msg / (sum_time / 1000.0 / 1000.0)) << " Msg/s" << std::endl;
  }
  std::cout << "--------------------------------------------" << std::endl;
}

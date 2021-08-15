#include <ecal/ecal.h>

#include <tclap/CmdLine.h>
#include <iostream>
#include <string>

void do_run(const int runs, int snd_size /*kB*/, int delay /*ms*/, bool zero_copy)
{
  // log parameter
  std::cout << "--------------------------------------------"    << std::endl;
  std::cout << "Runs                    : " << runs              << std::endl;
  std::cout << "Message size            : " << snd_size << " kB" << std::endl;
  std::cout << "Message delay           : " << delay    << " ms" << std::endl;
  if (zero_copy)
  {
    std::cout << "Zero copy               : ON" << std::endl;
  }

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "latency_snd");

  // create publisher and subscriber
  eCAL::CPublisher pub("ping");
  
  // enable zero copy mode
  pub.EnableZeroCopy(zero_copy);

  // prepare send buffer
  std::vector<char> snd_array(snd_size * 1024);

  // let them match
  eCAL::Process::SleepMS(2000);

  int run(0);
  for (run = 0; run < runs; ++run)
  {
    // get time and send message
    pub.Send(snd_array.data(), snd_array.size(), eCAL::Time::GetMicroSeconds());
    // delay
    eCAL::Process::SleepMS(delay);
  }

  // log test
  std::cout << "Messages sent           : " << run               << std::endl;
  std::cout << "--------------------------------------------"    << std::endl;

  // finalize eCAL API
  eCAL::Finalize();
}

int main(int argc, char **argv)
{
  try
  {
    // parse command line
    TCLAP::CmdLine cmd("latency_snd");
    TCLAP::ValueArg<int> runs     ("r", "runs",      "Number of messages to send.",           false, 1000, "int");
    TCLAP::ValueArg<int> size     ("s", "size",      "Messages size in kB.",                  false, 1,    "int");
    TCLAP::ValueArg<int> delay    ("d", "delay",     "Delay between two publications in ms.", false, 10,   "int");
    TCLAP::SwitchArg     zero_copy("z", "zero_copy", "Zero copy mode on/off.");
    cmd.add(runs);
    cmd.add(size);
    cmd.add(zero_copy);
    cmd.add(delay);
    cmd.parse(argc, argv);

    // run test
    do_run(runs.getValue(), size.getValue(), delay.getValue(), zero_copy.getValue());
  }
  // catch tclap exceptions
  catch (TCLAP::ArgException &e)
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return EXIT_FAILURE;
  }

  return(0);
}

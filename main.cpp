#include <iostream>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <future>

#include "bulkmt.h"
#include "commands_handler.h"

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: bulk N (N - amount of command for accumulation)" << std::endl;
    return 1;
  }
  std::string size_of_block_str(argv[1]);


  if (!std::all_of(size_of_block_str.begin(), size_of_block_str.end(), ::isdigit))
  {
    std::cout << "Incorrect input args!" << std::endl;
    return 1;
  }

  uint32_t size_of_block = boost::lexical_cast<int>(argv[1]);  

  std::mutex cv_m;
  CommandsHandler commands_handler(size_of_block);
  bool log_flag = false;
  bool file_flag = false;  
  const uint32_t log_threads_counter = 1;
  const uint32_t file_threads_counter = 2;  

  std::vector<std::thread> threads;
  std::vector<std::future<bulkmt::InfoStruct>> threads_futures;
  for(uint32_t i = 0; i < log_threads_counter + file_threads_counter; ++i)
  {
      std::promise<bulkmt::InfoStruct> promise;
      threads_futures.push_back(promise.get_future());
      threads.push_back(std::thread((i < log_threads_counter ? bulkmt::logger : bulkmt::file_writer), std::ref(commands_handler), std::ref(cv_m), std::ref(log_flag), std::ref(file_flag), std::move(promise)));
  }

  bulkmt::InfoStruct main_info_struct;

  std::string input_line;
  while (std::getline(std::cin, input_line))
  {
    main_info_struct.lines_counter++;

    bool notify_required = false;
    {
      std::lock_guard<std::mutex> lck(cv_m);
      notify_required = commands_handler.is_notify_required(input_line);
    }
    if (notify_required)
    {
      main_info_struct.commands_counter += commands_handler.get_commands_number();
      main_info_struct.blocks_counter += commands_handler.get_blocks_number();
      file_flag = log_flag = true;     
      bulkmt::cv_logger.notify_one();
      bulkmt::cv_file.notify_one();
    }
  }
  main_info_struct.commands_counter += commands_handler.get_commands_number();
  main_info_struct.blocks_counter += commands_handler.get_blocks_number();
  bulkmt::quit = file_flag = log_flag = true; 
  bulkmt::cv_logger.notify_all();
  bulkmt::cv_file.notify_all();

  for(auto &thr : threads)
  {
    thr.join();
  }
  std::cout << "main thread: " << main_info_struct.lines_counter << " lines, " << main_info_struct.commands_counter << " commands, " << main_info_struct.blocks_counter << " blocks" << std::endl;

  std::vector<bulkmt::InfoStruct> info_vector;
  for(auto &f : threads_futures)
  {
    info_vector.push_back(f.get());
  }
  for(uint32_t i = 0; i < log_threads_counter + file_threads_counter; ++i)
  { 
    std::cout << (i < log_threads_counter ? "log thread " : "file thread ") << (i < log_threads_counter ? i + 1 : i - log_threads_counter + 1)
     << ": " << info_vector[i].blocks_counter << " blocks, " << info_vector[i].commands_counter << " commands" << std::endl;
  }

  return 0;
}
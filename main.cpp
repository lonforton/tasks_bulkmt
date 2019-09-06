#include <iostream>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <future>

#include "bulkmt.h"
#include "commands_handler.h"
#include "commands_queue.h"

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

  size_t size_of_block = boost::lexical_cast<int>(argv[1]);  

  CommandsHandler commands_handler(size_of_block);

  std::queue<CommandsQueue> logger_queue;
  std::queue<CommandsQueue> file_queue;

  std::mutex logger_m;  
  std::mutex file_m;  

  const size_t log_threads_counter = 1;
  const size_t file_threads_counter = 2;  

  std::vector<std::thread> threads;
  std::vector<std::future<bulkmt::InfoStruct>> threads_futures;
  for(uint32_t i = 0; i < log_threads_counter + file_threads_counter; ++i)
  {
      std::promise<bulkmt::InfoStruct> promise;
      threads_futures.push_back(promise.get_future());
      if(i < log_threads_counter) {
        threads.emplace_back(std::thread(bulkmt::logger, std::ref(logger_queue), std::ref(logger_m), std::move(promise)));
      }
      else {
        threads.emplace_back(std::thread(bulkmt::file_writer, std::ref(file_queue), std::ref(file_m), std::move(promise)));
      }
  }

  bulkmt::InfoStruct main_info_struct;

  std::string input_line;
  while (std::getline(std::cin, input_line))
  {    
    main_info_struct.lines_counter++; 
    {
      std::scoped_lock<std::mutex, std::mutex> lock(logger_m, file_m); 
      commands_handler.add_input(input_line);
    }            
    
    if(commands_handler.is_notify_required()) {
      {
        std::scoped_lock<std::mutex, std::mutex> lock(logger_m, file_m); 
        logger_queue.emplace(commands_handler.get_commands());
        file_queue.emplace(commands_handler.get_commands());
        main_info_struct.commands_counter += commands_handler.get_commands_size();
        main_info_struct.blocks_counter += 1;
        commands_handler.clear_commands_queue();        
      }
      bulkmt::cv_logger.notify_one();
      bulkmt::cv_file.notify_one();
      commands_handler.notification_done();
    }
  }

  {
    std::scoped_lock<std::mutex, std::mutex> lock(logger_m, file_m);    
    commands_handler.close_input();   
    auto commands = commands_handler.get_commands();
    if(!commands.get_command().empty()) {
      logger_queue.emplace(commands_handler.get_commands());
      file_queue.emplace(commands_handler.get_commands());
      main_info_struct.commands_counter += commands_handler.get_commands_size();
      main_info_struct.blocks_counter += 1;
    }
  } 

  bulkmt::quit = true; 

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
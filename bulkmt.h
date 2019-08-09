#include <mutex>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cctype>
#include <vector>
#include <deque>

#include "commands_handler.h"

namespace bulkmt {

struct InfoStruct
{
  uint32_t lines_counter = 0;
  uint32_t commands_counter = 0;
  uint32_t blocks_counter = 0;
};

std::condition_variable cv_logger;
std::condition_variable cv_file;
bool quit = false;

void logger(CommandsHandler &commands_handler, std::mutex &cv_m, bool &log_flag, bool &file_flag, std::promise<InfoStruct> &&info)
{
  InfoStruct info_struct;
  while (!quit)
  {
    std::unique_lock<std::mutex> lk(cv_m);
    cv_logger.wait(lk, [&log_flag]() { return log_flag || quit; });    

    std::cout << "bulk: " << commands_handler.get_commands_string() << std::endl;
    info_struct.commands_counter += commands_handler.get_commands_number();
    info_struct.blocks_counter += commands_handler.get_blocks_number();    

    lk.unlock();
    
    log_flag = false;
    if (!log_flag && !file_flag)
    {
      commands_handler.clear_commands();
    }
  }
  info.set_value(info_struct);
}

void file_writer(CommandsHandler &commands_handler, std::mutex &cv_m, bool &log_flag, bool &file_flag, std::promise<InfoStruct> &&info)
{
  InfoStruct info_struct;
  while (!quit)
  {
    std::unique_lock<std::mutex> lk(cv_m);
    cv_file.wait(lk, [&file_flag]() { return file_flag || quit; });

    std::ofstream bulk_file;
    bulk_file.open(commands_handler.get_first_command_timestamp() + "_" + std::to_string(commands_handler.get_unique_file_id()) + ".log");
    bulk_file << commands_handler.get_commands_string() << std::endl;
    bulk_file.close();

    info_struct.commands_counter += commands_handler.get_commands_number();
    info_struct.blocks_counter += commands_handler.get_blocks_number();  

    lk.unlock();

    file_flag = false;
    if (!log_flag && !file_flag)
    {
      commands_handler.clear_commands();
    }
  }
  info.set_value(info_struct);
}

} // namespace bulkmt
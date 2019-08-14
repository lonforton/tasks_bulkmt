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

std::string parse_command(std::vector<std::string> command)
{
  std::ostringstream commands_line;

  if (!command.empty())
  {
     std::copy(command.begin(), command.end() - 1,
               std::ostream_iterator<std::string>(commands_line, " "));
     commands_line << command.back();
  }

  return commands_line.str();
}

void logger(CommandsHandler &commands_handler, std::mutex &cv_m, bool &logger_read, bool &file_read, std::promise<InfoStruct> &&info)
{
  InfoStruct info_struct;
  while (!quit || !commands_handler.is_commands_queue_empty())
  {
    std::unique_lock<std::mutex> lk(cv_m);
    cv_logger.wait(lk, [&]() { return (!commands_handler.is_commands_queue_empty() && !logger_read) || quit; });     
   
    if(quit && logger_read) 
      continue;

    if(!commands_handler.is_commands_queue_empty()) {
      auto com = commands_handler.get_next_command();
      std::string commands_string = parse_command(com.first);

      std::cout << "bulk: " << commands_string << std::endl;
      info_struct.commands_counter += com.first.size();
      info_struct.blocks_counter += com.second ? 1 : 0;
    }
    
    logger_read = true;
    if (file_read)
    {
      logger_read = file_read = false;
      commands_handler.pop_commands_queue();
    }    

  }
  info.set_value(info_struct);
}

void file_writer(CommandsHandler &commands_handler, std::mutex &cv_m, bool &logger_read, bool &file_read, std::promise<InfoStruct> &&info)
{
  InfoStruct info_struct;
  while (!quit || !commands_handler.is_commands_queue_empty())
  {
    std::unique_lock<std::mutex> lk(cv_m);
    cv_file.wait(lk, [&]() { return (!commands_handler.is_commands_queue_empty() && !file_read) || quit; });

    if(quit && file_read) 
      continue;

    if(!commands_handler.is_commands_queue_empty()) {
      auto com = commands_handler.get_next_command();
      std::string commands_string = parse_command(com.first);

      std::ofstream bulk_file;
      bulk_file.open(commands_handler.get_first_command_timestamp() + "_" + std::to_string(commands_handler.get_unique_file_id()) + ".log");
      bulk_file << commands_string << std::endl;
      bulk_file.close();

      info_struct.commands_counter += com.first.size();
      info_struct.blocks_counter += com.second ? 1 : 0;
    }

    //lk.unlock();

    file_read = true;
    if (logger_read)
    {
      logger_read = file_read = false;
      commands_handler.pop_commands_queue();
    }
  }
  info.set_value(info_struct);
}

} // namespace bulkmt
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
#include "commands_queue.h"

namespace bulkmt {

struct InfoStruct
{
  uint32_t lines_counter = 0;
  uint32_t commands_counter = 0;
  uint32_t blocks_counter = 0;
};

std::condition_variable cv_logger;
std::condition_variable cv_file;
std::atomic_bool quit = false;

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

void logger(std::queue<CommandsQueue> &queue, std::mutex &cv_m, std::promise<InfoStruct> &&info)
{
   InfoStruct info_struct;
   bool commands_queue_empty = false;
   while (!quit.load() || !commands_queue_empty)
   {     
     std::unique_lock<std::mutex> lk(cv_m);
     cv_logger.wait(lk, [&]() { return (!queue.empty() || quit.load()); });    
         
    if(queue.empty()) {
       commands_queue_empty = true;
       continue;
     }     

     auto com = queue.front().get_command();
     std::string commands_string = parse_command(com);
     queue.pop();

     lk.unlock();

     std::cout << "bulk: " << commands_string << std::endl;
     info_struct.commands_counter += com.size();   
     info_struct.blocks_counter += 1;
     
   }
   info.set_value(info_struct);
}

void file_writer(std::queue<CommandsQueue> &queue, std::mutex &cv_m, std::promise<InfoStruct> &&info)
{
   InfoStruct info_struct;
   bool commands_queue_empty = false;
   while (!quit.load() || !commands_queue_empty)
   {
     std::unique_lock<std::mutex> lk(cv_m);
     cv_file.wait(lk, [&]() { return (!queue.empty() || quit.load()); });
     if(queue.empty()) {
       commands_queue_empty = true;
       continue;
     }

     auto com = queue.front().get_command();
     std::string commands_string = parse_command(com);
          
     std::string file_name(queue.front().get_first_command_timestamp() + "_" + std::to_string(queue.front().get_unique_file_id()) + ".log");

     queue.pop();

     lk.unlock();

     std::ofstream bulk_file;
     bulk_file.open(file_name);
     bulk_file << commands_string << std::endl;
     bulk_file.close();

     info_struct.commands_counter += com.size();
     info_struct.blocks_counter += 1;     
   }
   info.set_value(info_struct);
}

} // namespace bulkmt
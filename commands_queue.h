#ifndef COMMANDS_QUEUE_H
#define COMMANDS_QUEUE_H

#include <queue>
#include <string>
#include <vector>

class CommandsQueue 
{
  public:

    CommandsQueue(std::vector<std::string> command, std::string first_command_time, unsigned int unique_file_id) :
    _command(command), _first_command_time(first_command_time), _unique_file_id(unique_file_id) 
    {

    }

    std::vector<std::string>  get_command() {
      return _command;
    }

    std::string get_first_command_timestamp() {
      return _first_command_time;
    }

    unsigned int get_unique_file_id() {
      return _unique_file_id;
    }


  private:
    std::vector<std::string> _command;
    std::string _first_command_time;
    unsigned int _unique_file_id;     
};

#endif 
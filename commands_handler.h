#ifndef COMMANDS_HANDLER_H
#define COMMANDS_HANDLER_H

#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cctype>
#include <vector>
#include <queue>

#include "commands_queue.h"

class CommandsHandler
{
public:
  CommandsHandler(int size_of_block) : _size_of_block(size_of_block)
  {
    unique_file_id = 0;
  }

  void add_input(const std::string &input_line)
  {
    switch (map_command(input_line))
    {
    case opening_bracket:
      if (++brackets_counter == 1 && _commands.size() > 0)
      {
        _notify_required = true;
      }
      break;

    case closing_bracket:
      if (--brackets_counter <= 0)
      {
        brackets_counter = 0;
        _notify_required = true;
      }
      break;

    default:
      if (is_first_command())
      {
        _first_command_time = std::chrono::system_clock::now();
      }

      _commands.push_back(input_line);

      if (_commands.size() >= _size_of_block && brackets_counter <= 0)
      {
        _notify_required = true;
      }
      break;
    }
  }


  CommandsQueue get_commands()
  {   
    return CommandsQueue(_commands, get_first_command_time(), get_unique_file_id());
  }

  size_t get_commands_size()
  {
    return _commands.size();
  }

  void clear_commands_queue()
  {
     _commands.clear();
  }

  bool is_first_command() const
  {
    return _commands.size() == 0;
  }

  bool is_notify_required() const
  {
    return _notify_required;
  }

  void notification_done()
  {
    _notify_required = false;   
    update_unique_file_id();
  }

  unsigned int get_unique_file_id() 
  {
    return unique_file_id;
  }

  void update_unique_file_id() {
     unique_file_id.exchange(++unique_file_id);
  }

  uint32_t map_command(const std::string &command) const
  {
    if (command.front() == '{')
      return opening_bracket;
    if (command.front() == '}')
      return closing_bracket;
    return other_command;
  }

  std::chrono::time_point<std::chrono::system_clock> get_first_command_time() const
  {
    return _first_command_time;
  }

  void close_input() 
  {
    add_input(std::string("{"));
  }

  void get_stream_input(std::istream &is) 
  {
    std::string input_line;
    while (std::getline(is, input_line))
    {
      add_input(input_line);
    }
    close_input();
  }

private:
  std::vector<std::string> _commands;
  unsigned int _size_of_block = 0;
  int brackets_counter = 0;
  std::chrono::time_point<std::chrono::system_clock> _first_command_time;
  enum
  {
    opening_bracket = 0,
    closing_bracket = 1,
    other_command = 2
  };
  std::atomic_uint unique_file_id;
  bool _notify_required = false;
};

#endif
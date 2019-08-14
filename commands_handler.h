#ifndef COMMANDS_HANDLER_H
#define COMMANDS_HANDLER_H

#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cctype>
#include <vector>
#include <queue>

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
        _commands_queue.push(std::make_pair(_commands, block_of_commands));
        _commands.clear();
        _notify_required = true;
      }
      break;

    case closing_bracket:
      if (--brackets_counter <= 0)
      {
        brackets_counter = 0;
        block_of_commands = true;
        _commands_queue.push(std::make_pair(_commands, block_of_commands));
        _commands.clear();
        _notify_required = true;
      }
      break;

    default:
      if (is_first_command())
      {
        first_command_time = std::chrono::system_clock::now();
      }

      _commands.push_back(input_line);

      if (_commands.size() >= _size_of_block && brackets_counter <= 0)
      {
        _commands_queue.push(std::make_pair(_commands, block_of_commands));
        _commands.clear();
        _notify_required = true;
      }
      break;
    }
  }

  bool is_first_command() const
  {
    return _commands.size() == 0;
  }

  bool is_commands_queue_empty() const
  {
    return _commands_queue.size() == 0;
  }

  void pop_commands_queue()
  {
    if(!_commands_queue.empty())
    {
      _commands_queue.pop();
    }
  }

  bool is_notify_required() const
  {
    return _notify_required;
  }

  void notification_done()
  {
    _notify_required = false;
     block_of_commands = false;
  }

  std::pair<std::vector<std::string>, bool> get_next_command() const
  {
    if(!_commands_queue.empty()) {
      return _commands_queue.front();
    }

    return std::pair<std::vector<std::string>, bool>{};
  }

  uint32_t get_current_commands_counter() const
  {
    return _commands_queue.back().first.size();
  }

  uint32_t get_commands_number() const
  {
    return get_next_command().first.size();
  }

  uint32_t get_blocks_number() const
  {
    return _commands_queue.back().second;
  }

  unsigned int get_unique_file_id() 
  {
    return unique_file_id.exchange(++unique_file_id);
  }

  uint32_t map_command(const std::string &command) const
  {
    if (command.front() == '{')
      return opening_bracket;
    if (command.front() == '}')
      return closing_bracket;
    return other_command;
  }

  std::string get_first_command_timestamp() const
  {
    auto duration = first_command_time.time_since_epoch();
    return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
  }

  void close_input() 
  {
    add_input(std::string("{"));
    _notify_required = false;
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
  std::chrono::time_point<std::chrono::system_clock> first_command_time;
  enum
  {
    opening_bracket = 0,
    closing_bracket = 1,
    other_command = 2
  };
  bool block_of_commands = false;
  std::atomic_uint unique_file_id;
  std::queue< std::pair< std::vector<std::string>, bool> > _commands_queue;
  bool _notify_required = false;
};

#endif
#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cctype>
#include <vector>

class CommandsHandler
{
public:
  CommandsHandler(int size_of_block) : _size_of_block(size_of_block)
  {
    unique_file_id = 0;
  }

  bool is_notify_required(const std::string &input_line)
  {
    switch (map_command(input_line))
    {
    case opening_bracket:
      if (++brackets_counter == 1 && _commands.size() > 0)
      {
        return true;
      }
      return false;
      break;

    case closing_bracket:
      if (--brackets_counter <= 0)
      {
        brackets_counter = 0;
        block_of_commands = true;
        return true;
      }
      return false;
      break;

    default:
      if (is_first_command())
      {
        first_command_time = std::chrono::system_clock::now();
      }
      _commands.push_back(input_line);

      if (_commands.size() >= _size_of_block && brackets_counter <= 0)
      {
        return true;
      }
      return false;
      break;
    }

    return false;
  }

  void clear_commands()
  {
    _commands.clear();
    block_of_commands = false;
  }

  bool is_first_command() const
  {
    return _commands.size() == 0;
  }

  uint32_t get_commands_number()
  {
    return _commands.size();
  }

  uint32_t get_blocks_number()
  {
    if (block_of_commands)
      return 1;

    return 0;
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

  std::string get_commands_string() const
  {
    std::ostringstream commands_line;

    if (!_commands.empty())
    {
      std::copy(_commands.begin(), _commands.end() - 1,
                std::ostream_iterator<std::string>(commands_line, " "));
      commands_line << _commands.back();
    }
    return commands_line.str();
  }

  std::string get_first_command_timestamp() const
  {
    auto duration = first_command_time.time_since_epoch();
    return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
  }

  void get_input(const std::string &input_line)
  {
    if (is_notify_required(input_line))
    {
    }
  }

  void close_input()
  {
    get_input(std::string("{"));
  }

  void get_stream_input(std::istream &is)
  {
    std::string input_line;
    while (std::getline(is, input_line))
    {
      get_input(input_line);
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
};

#endif
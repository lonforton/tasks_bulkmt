#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cctype>

class CommandsHandler
{
public:
  CommandsHandler(int size_of_block) : _size_of_block(size_of_block)
  {
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
  }

  bool is_first_command() const
  {
    return _commands.size() == 0;
  }

  uint32_t map_command(const std::string& command) const
  {
    if(command.front() == '{') return opening_bracket;
    if(command.front() == '}') return closing_bracket;
    return other_command;
  }

  std::string getCommandString() const
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

  std::string getFirstCommandTimeStamp() const
  {
    auto duration = first_command_time.time_since_epoch();
    return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
  }

private:
  std::vector<std::string> _commands;
  unsigned int _size_of_block = 0;
  int brackets_counter = 0;
  std::chrono::time_point<std::chrono::system_clock> first_command_time;
  enum { opening_bracket = 0, closing_bracket = 1,  other_command = 2 };
};

class Output
{
public:
  virtual ~Output() = default;
  virtual void update(const CommandsHandler &commands_handler) = 0;
};

class DisplayOutput : public Output
{
public:
  virtual ~DisplayOutput() = default;
  void update(const CommandsHandler &commands_handler)
  {
    std::cout << "bulk: " << commands_handler.getCommandString() << std::endl;
  }
};

class FileOutput : public Output
{
public:
  virtual ~FileOutput() = default;
  void update(const CommandsHandler &commands_handler)
  {
    std::ofstream bulk_file;
    bulk_file.open(commands_handler.getFirstCommandTimeStamp() + ".log");
    bulk_file << commands_handler.getCommandString() << std::endl;
    bulk_file.close();
  }
};

class Notifier
{

public:
  Notifier(int size_of_block) : _command_handler(size_of_block)
  {
  }

  void subscribe(Output *output)
  {
    _outputs.push_back(output);
  }

  void notify_all() const
  {
    for (const auto &output : _outputs)
    {
      output->update(_command_handler);
    }
  }

  void get_input(const std::string &input_line)
  {
    if (_command_handler.is_notify_required(input_line))
    {
      notify_all();
      _command_handler.clear_commands();
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
  CommandsHandler _command_handler;
  std::vector<Output *> _outputs;
};
#define BOOST_TEST_MODULE bulk_test_module

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
#include <iostream>

#include "bulkmt.h"

BOOST_AUTO_TEST_SUITE(bulkmt_test_suite)

/**
 * @brief Helper redirection fot tests
 */
struct cout_redirect
{
  cout_redirect(std::streambuf *new_buffer)
      : old(std::cout.rdbuf(new_buffer))
  {
  }

  ~cout_redirect()
  {
    std::cout.rdbuf(old);
  }

private:
  std::streambuf *old;
};

BOOST_AUTO_TEST_CASE(bulkmt_input_test_1)
{
  boost::test_tools::output_test_stream output;
  {
    cout_redirect redirect(output.rdbuf());

    Notifier notifier(2);

    auto display_output = std::make_unique<DisplayOutput>(); 

    notifier.subscribe(display_output.get());

    std::stringstream ss;
    ss << std::string("1") << std::endl
       << std::string("2") << std::endl
       << std::string("3") << std::endl
       << std::string("4") << std::endl;

    notifier.get_stream_input(ss);
  }
  BOOST_CHECK(output.is_equal("bulk: 1 2\nbulk: 3 4\n"));
}

BOOST_AUTO_TEST_CASE(bulkmt_input_test_2)
{
  boost::test_tools::output_test_stream output;
  {
    cout_redirect redirect(output.rdbuf());

    Notifier notifier(1);

    auto display_output = std::make_unique<DisplayOutput>(); 

    notifier.subscribe(display_output.get());

    std::stringstream ss;
    ss << std::string("1") << std::endl
       << std::string("2") << std::endl
       << std::string("3") << std::endl
       << std::string("4") << std::endl;       

    notifier.get_stream_input(ss);
  }
  BOOST_CHECK(output.is_equal("bulk: 1\nbulk: 2\nbulk: 3\nbulk: 4\n"));
}

BOOST_AUTO_TEST_CASE(bulkmt_input_test_3)
{
  boost::test_tools::output_test_stream output;
  {
    cout_redirect redirect(output.rdbuf());

    Notifier notifier(3);

    auto display_output = std::make_unique<DisplayOutput>(); 

    notifier.subscribe(display_output.get());

    std::stringstream ss;
    ss << std::string("1") << std::endl
       << std::string("2") << std::endl
       << std::string("3") << std::endl
       << std::string("{") << std::endl
       << std::string("4") << std::endl
       << std::string("5") << std::endl
       << std::string("6") << std::endl
       << std::string("7") << std::endl
       << std::string("}") << std::endl;

    notifier.get_stream_input(ss);
  }
  BOOST_CHECK(output.is_equal("bulk: 1 2 3\nbulk: 4 5 6 7\n"));
}

BOOST_AUTO_TEST_CASE(bulkmt_input_test_4)
{
  boost::test_tools::output_test_stream output;
  {
    cout_redirect redirect(output.rdbuf());

    Notifier notifier(3);

    auto display_output = std::make_unique<DisplayOutput>(); 

    notifier.subscribe(display_output.get());

    std::stringstream ss;
    ss << std::string("{") << std::endl
       << std::string("1") << std::endl
       << std::string("2") << std::endl
       << std::string("{") << std::endl
       << std::string("3") << std::endl
       << std::string("4") << std::endl
       << std::string("}") << std::endl
       << std::string("5") << std::endl
       << std::string("6") << std::endl
       << std::string("}") << std::endl;

    notifier.get_stream_input(ss);
  }
  BOOST_CHECK(output.is_equal("bulk: 1 2 3 4 5 6\n"));
}

BOOST_AUTO_TEST_CASE(bulkmt_input_test_5)
{
  boost::test_tools::output_test_stream output;
  {
    cout_redirect redirect(output.rdbuf());

    Notifier notifier(3);

    auto display_output = std::make_unique<DisplayOutput>(); 

    notifier.subscribe(display_output.get());

    std::stringstream ss;
    ss << std::string("1") << std::endl
       << std::string("2") << std::endl
       << std::string("3") << std::endl
       << std::string("{")  << std::endl
       << std::string("4") << std::endl
       << std::string("5") << std::endl
       << std::string("6") << std::endl
       << std::string("7") << std::endl;
    notifier.get_stream_input(ss);
  }
  BOOST_CHECK(output.is_equal("bulk: 1 2 3\n"));
}

BOOST_AUTO_TEST_CASE(bulkmt_input_test_6)
{
  boost::test_tools::output_test_stream output;
  {
    cout_redirect redirect(output.rdbuf());

    Notifier notifier(3);

    auto display_output = std::make_unique<DisplayOutput>(); 

    notifier.subscribe(display_output.get());

    std::stringstream ss;
    ss << std::string("0") << std::endl
       << std::string("1") << std::endl
       << std::string("2") << std::endl
       << std::string("3") << std::endl
       << std::string("4") << std::endl
       << std::string("5") << std::endl
       << std::string("6") << std::endl
       << std::string("7") << std::endl
       << std::string("8") << std::endl
       << std::string("9") << std::endl;

    notifier.get_stream_input(ss);
  }
  BOOST_CHECK(output.is_equal("bulk: 0 1 2\nbulk: 3 4 5\nbulk: 6 7 8\nbulk: 9\n"));
}

BOOST_AUTO_TEST_CASE(bulkmt_input_test_7)
{
  boost::test_tools::output_test_stream output;
  {
    cout_redirect redirect(output.rdbuf());

    Notifier notifier(3);

    auto display_output = std::make_unique<DisplayOutput>(); 

    notifier.subscribe(display_output.get());

    std::stringstream ss;
    ss << std::string("0") << std::endl
       << std::string("1") << std::endl
       << std::string("2") << std::endl
       << std::string("3") << std::endl
       << std::string("4") << std::endl
       << std::string("{") << std::endl
       << std::string("00") << std::endl
       << std::string("}") << std::endl;

    notifier.get_stream_input(ss);
  }
  BOOST_CHECK(output.is_equal("bulk: 0 1 2\nbulk: 3 4\nbulk: 00\n"));
}

BOOST_AUTO_TEST_CASE(bulkmt_input_test_8)
{
  boost::test_tools::output_test_stream output;
  {
    cout_redirect redirect(output.rdbuf());

    Notifier notifier(2);

    auto display_output = std::make_unique<DisplayOutput>(); 

    notifier.subscribe(display_output.get());

    std::stringstream ss;
    ss << std::string("0") << std::endl
       << std::string("") << std::endl
       << std::string("") << std::endl
       << std::string("1") << std::endl
       << std::string("") << std::endl
       << std::string("") << std::endl
       << std::string("2") << std::endl
       << std::string("3") << std::endl;

    notifier.get_stream_input(ss);
  }
  BOOST_CHECK(output.is_equal("bulk: 0 \nbulk:  1\nbulk:  \nbulk: 2 3\n"));
}

BOOST_AUTO_TEST_SUITE_END()

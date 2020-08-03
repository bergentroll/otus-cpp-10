#include <gtest/gtest.h>
#include <limits>

#include "get_arg.hpp"
#include "logger.hpp"
#include "parser.hpp"

using namespace std;
using namespace otus;

class MockLogger: public ILogger {
public: 
  virtual void print(string const &input, unsigned) { ss << input; }

  virtual void setMainStatistics(string_view) { }

  string str() const {
    return ss.str();
  }

  void str(string const &input) {
    ss.str(input);
  }

private:
  stringstream ss { };

};

TEST(bulk, plain) {
  MockLogger l { };
  Parser p { 2, l };

  p << "cmd1" << "cmd2";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2\n");
  l.str("");

  p << "cmd1" << "cmd2";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2\n");
}

TEST(bulk, block) {
  MockLogger l { };
  Parser p { 2, l };

  p << "cmd1" << "cmd2";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2\n");
  l.str("");

  p << "{" << "cmd1" << "cmd2" << "cmd3" << "}";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2, cmd3\n");
  l.str("");

  p << "cmd1" << "cmd2";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2\n");
}

TEST(bulk, nested_blocks) {
  MockLogger l { };
  Parser p { 2, l };

  p << "cmd1" << "cmd2";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2\n");
  l.str("");

  p << "{" << "cmd1" << "{" << "cmd2" << "}" << "cmd3" << "}";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2, cmd3\n");
  l.str("");

  p
    << "{" << "cmd1"
    << "{" << "cmd2"
    << "{" << "cmd3" << "}" << "}"
    << "cmd4" << "}";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2, cmd3, cmd4\n");
  l.str("");

  p << "cmd1" << "cmd2";
  EXPECT_EQ(l.str(), "bulk: cmd1, cmd2\n");
}

TEST(bulk, invalid_token) {
  MockLogger l { };
  Parser p { 2, l };

  p << "cmd1" << "{" << "cmd2" << "}";
  EXPECT_THROW(p << "}", Parser::InvalidToken);
}

TEST(get_arg, valid) {
  char const *v[] { "bulk", "345" };
  EXPECT_EQ(345, get_arg(2, v));
}

TEST(get_arg, invalid_argc) {
  char const *v1[] { "bulk" };
  EXPECT_THROW(get_arg(1, v1), InvalidArgument);

  char const *v2[] { "bulk", "345", "-h" };
  EXPECT_THROW(get_arg(3, v2), InvalidArgument);
}

TEST(get_arg, invalid_arg) {
  char const *v[] { "bulk", "a345" };
  EXPECT_THROW(get_arg(2, v), InvalidArgument);

  v[1] = "3a45";
  EXPECT_THROW(get_arg(2, v), InvalidArgument);

  v[1] = "345a";
  EXPECT_THROW(get_arg(2, v), InvalidArgument);
}

TEST(get_arg, out_of_range) {
  char const *v[] { "bulk", "-345" };
  EXPECT_THROW(get_arg(2, v), InvalidArgument);

  v[1] = to_string(static_cast<long>(numeric_limits<int>::max()) + 1).c_str();
  EXPECT_THROW(get_arg(2, v), InvalidArgument);
}

#include <iostream>
#include <memory>

#include "get_arg.hpp"
#include "parser.hpp"
#include "logger.hpp"

using namespace std;
using namespace otus;

int main(int argc, char const *argv[]) {
  int N;
  try {
    N = get_arg(argc, argv);
  } catch (InvalidArgument &e) {
    cerr << "Usage error: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  Logger logger { };
  Parser parser { N, logger };
  string buf { };

  cin >> buf;
  while (cin) {
    try {
      parser << buf;
    }
    catch (Parser::InvalidToken &e) {
      cerr << "Error occured: " << e.what() << endl;
    }
    cin >> buf;
  }

  return EXIT_SUCCESS;
}

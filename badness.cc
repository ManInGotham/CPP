// This program takes path to a file as a runtime parameter.  It opens the
// file, reads at most the first 40 bytes, and prints them to the console as
// a quoted string.
//
// Except that it doesn't do that because the program is buggy.  It compiles
// without issues but it hangs every time it runs and never prints anything.
// What's wrong with it?

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <unistd.h>

struct fd_t {

  fd_t()
      : handle(-1) {}

  fd_t(int new_handle)
      : handle(new_handle) {
    if (new_handle < 0) {
      throw std::runtime_error { "open failed" };
    }
  }

  ~fd_t() {
    if (*this) {
      close(handle);
    }
  }

  explicit operator bool() const {
    return handle >= 0;
  }

  int operator*() const {
    return handle;
  }

  int handle;

};  // fd_t

std::string read_some(const char *path, size_t max_size) {
  fd_t fd { open(path, O_RDONLY) };
  std::string result(max_size, 'x');
  auto actual_size =
      read(*fd, const_cast<char *>(result.data()), result.size());
  if (actual_size < 0) {
    throw std::runtime_error { "read failed" };
  }
  result.resize(actual_size);
  return result;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    throw std::runtime_error { "usage: badness <path>" };
  }
  std::cout << std::quoted(read_some(argv[1], 40).c_str()) << std::endl;
  return 0;
}

#include <err.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>

#include <sys/socket.h>
#include <sys/un.h>

int main() {
  const auto socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    err(EXIT_FAILURE, "socket");
  }

  struct sockaddr_un address;
  memset(&address, 0, sizeof(address));
  address.sun_family = AF_UNIX;
  strncpy(address.sun_path, "/tmp/socket.sock", sizeof(address.sun_path) - 1);

  if (bind(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
    err(EXIT_FAILURE, "bind");
  }

  if (listen(socket_fd, 10) == -1) {
    err(EXIT_FAILURE, "listen");
  }

  const auto connection_fd = accept(socket_fd, nullptr, nullptr);
  if (connection_fd == -1) {
    err(EXIT_FAILURE, "accept");
  }

  char buffer[1024];
  const auto buffer_size = read(connection_fd, buffer, sizeof(buffer));
  if (buffer_size == -1) {
    err(EXIT_FAILURE, "read");
  }

  if (write(connection_fd, buffer, buffer_size) == -1) {
    err(EXIT_FAILURE, "write");
  }

  if (close(connection_fd) == -1) {
    err(EXIT_FAILURE, "close");
  }

  if (close(socket_fd) == -1) {
    err(EXIT_FAILURE, "close");
  }

  if (unlink("/tmp/socket.sock") == -1) {
    err(EXIT_FAILURE, "unlink");
  }
}

#include <err.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include <sys/socket.h>
#include <sys/un.h>

#include <atomic>

std::atomic<bool> stop = false;

void handleSignal(const int) {
  stop = true;
}

int main() {
  struct sigaction signal_action;
  signal_action.sa_handler = handleSignal;
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = 0;

  sigaction(SIGINT, &signal_action, nullptr);
  sigaction(SIGTERM, &signal_action, nullptr);

  const auto socket_family = AF_UNIX;
  const auto socket_fd = socket(socket_family, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    err(EXIT_FAILURE, "socket");
  }

  struct sockaddr_un address;
  memset(&address, 0, sizeof(address));
  address.sun_family = socket_family;

  const auto socket_path = "/tmp/socket.sock";
  strncpy(address.sun_path, socket_path, sizeof(address.sun_path) - 1);

  if (bind(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
    err(EXIT_FAILURE, "bind");
  }

  if (listen(socket_fd, 10) == -1) {
    err(EXIT_FAILURE, "listen");
  }

  while (!stop) {
    const auto connection_fd = accept(socket_fd, nullptr, nullptr);
    if (connection_fd == -1) {
      if (errno == EINTR) {
        continue;
      }
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
  }

  if (close(socket_fd) == -1) {
    err(EXIT_FAILURE, "close");
  }

  if (unlink(socket_path) == -1) {
    err(EXIT_FAILURE, "unlink");
  }
}

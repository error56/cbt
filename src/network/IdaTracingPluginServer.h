#ifndef IDATRACINGPLUGINSERVER_H
#define IDATRACINGPLUGINSERVER_H

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <mutex>

constexpr uint64_t kDebugReceiveBufferSize = 128;

class IdaTracingPluginServer {
  asio::io_context io_context_;
  asio::ip::tcp::acceptor acceptor_;

  std::vector<std::shared_ptr<asio::ip::tcp::socket>> sockets_;
  std::mutex lock_;

  void HandleClient(
      const std::shared_ptr<asio::ip::tcp::socket>& socket) noexcept;

 public:
  IdaTracingPluginServer(const std::string& address, uint16_t port);
  ~IdaTracingPluginServer();
  IdaTracingPluginServer(const IdaTracingPluginServer&) = delete;
  IdaTracingPluginServer& operator=(const IdaTracingPluginServer&) = delete;

  void Start();
  void BroadcastNewInstruction(const std::string& file,
                               uint64_t rva) noexcept;
};

#endif  // IDATRACINGPLUGINSERVER_H

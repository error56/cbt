#include "IdaTracingPluginServer.h"

#include <asio/write.hpp>
#include <iostream>

#include "spdlog/spdlog.h"

IdaTracingPluginServer::IdaTracingPluginServer(const std::string& address,
                                               const uint16_t port)
    : acceptor_(io_context_,
                asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

void IdaTracingPluginServer::Start() {
  while (true) {
    try {
      spdlog::info("Debug server listening on: {}:{}",
                   acceptor_.local_endpoint().address().to_string(),
                   acceptor_.local_endpoint().port());

      auto socket = std::make_shared<asio::ip::tcp::socket>(io_context_);
      acceptor_.accept(*socket);

      spdlog::info("Debug client connected: {}",
                   socket->remote_endpoint().address().to_string());

      {
        std::lock_guard<std::mutex> lock(lock_);
        sockets_.push_back(socket);
      }

      std::thread(&IdaTracingPluginServer::HandleClient, this, socket).detach();
    } catch (const std::exception& e) {
      spdlog::error("Error {}", e.what());
    }
  }
}

void IdaTracingPluginServer::HandleClient(
    const std::shared_ptr<asio::ip::tcp::socket>& socket) noexcept {
  try {
    while (true) {
      std::array<uint8_t, kDebugReceiveBufferSize> data{};

      if (const std::size_t length = socket->read_some(asio::buffer(data));
          length > 0) {
        // Ignore it
      }
    }
  } catch (std::exception& e) {
    spdlog::warn("Debug client disconnected");

    // Remove client from list
    std::lock_guard lock(lock_);
    std::erase(sockets_, socket);
  }
}

void IdaTracingPluginServer::BroadcastNewInstruction(
    const std::string& file, const uint64_t rva) noexcept {
  const auto text = std::format("{}:0x{:x}", file, rva);
  const auto message = asio::buffer(text + "\n");

  std::lock_guard<std::mutex> lock(lock_);
  for (auto& socket : sockets_) {
    try {
      asio::write(*socket, message);
    } catch (std::exception& e) {
      spdlog::error("Error sending to a client: {}", e.what());
    }
  }
}

IdaTracingPluginServer::~IdaTracingPluginServer() {
  if (acceptor_.is_open()) {
    acceptor_.close();
  }

  std::lock_guard<std::mutex> lock(lock_);
  for (const auto& socket : sockets_) {
    if (socket && socket->is_open()) {
      std::error_code error_code;
      socket->shutdown(asio::ip::tcp::socket::shutdown_both, error_code);
      socket->close(error_code);
    }
  }
  sockets_.clear();
}

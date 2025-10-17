#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

#include "configs.h"
#include "dice.h"
#include "proto/game.pb.h"

namespace bg {

template <typename P>
class ProtoWriter {
 public:
  ProtoWriter(ProtoWriter &&) = default;
  auto operator=(ProtoWriter &&) noexcept -> ProtoWriter & = default;
  ProtoWriter() = default;
  ProtoWriter(const std::filesystem::path &path, size_t buffer_size) : buffer_size_(buffer_size) {
    file_stream_.emplace(path, std::ios::binary | std::ios::app);
    out_ = &file_stream_.value();
    buffer_.reserve(buffer_size_);
  }

  ProtoWriter(std::ostream *out, size_t buffer_size) : out_(out), buffer_size_(buffer_size) {
    buffer_.reserve(buffer_size_);
  }

  ~ProtoWriter() { flush(); }

  void flush() {
    for (const auto &item : buffer_) {
      std::string serialized = item.SerializeAsString();
      uint32_t size = serialized.size();
      out_->write(reinterpret_cast<const char *>(&size), sizeof(size));
      out_->write(serialized.data(), size);
    }
    out_->flush();
    buffer_.clear();
  }

  void add(P &&item) {
    buffer_.emplace_back(item);
    if (buffer_.size() >= buffer_size_) [[unlikely]] {
      flush();
    }
  }

 private:
  std::optional<std::ofstream> file_stream_;
  std::ostream *out_;
  size_t buffer_size_;
  std::vector<P> buffer_;
};

class LogWriter {
 public:
  LogWriter(LogWriter &&) = default;
  auto operator=(LogWriter &&) noexcept -> LogWriter & = default;
  LogWriter() = default;

  LogWriter(const std::filesystem::path &path, size_t buffer_size, bool is_dummy = false);
  LogWriter(std::ostream *out, size_t buffer_size, bool is_dummy = false);
  ~LogWriter();

  void start_game(uint64_t game_id);
  void add_move(Dice dice, const Board &b);
  void end_game(int32_t score);
  void flush();

 private:
  ProtoWriter<Game> writer_;
  bool is_dummy_;
  Game game_in_construction_;
};

class LogReader {
 public:
  explicit LogReader(std::istream &in) : in_(in) {}
  auto read_all() -> std::vector<Game>;

 private:
  std::istream &in_;
};

}  // namespace bg

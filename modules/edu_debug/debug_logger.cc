/******************************************************************************
 * Implementation of the shared EduDebugLogger.
 *****************************************************************************/

#include "modules/edu_debug/debug_logger.h"

#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <set>
#include <sstream>

#include <gflags/gflags.h>

#include "cyber/time/clock.h"
#include "modules/edu_debug/sha1.h"

namespace apollo {
namespace edu_debug {

namespace {
constexpr std::size_t kChunkBytes = 48u * 1024u;
constexpr std::size_t kMaxCacheBytes = 8u * 1024u * 1024u;
constexpr double kReplayIntervalSec = 0.25;
constexpr double kReplayCyclePauseSec = 20.0;
constexpr double kWatchIntervalSec = 5.0;

bool EndsWith(const std::string& value, const std::string& suffix) {
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
             0;
}

// Only startup snapshots are worth caching for replay; per-cycle states would
// grow the cache without bound.
bool IsCachedKind(const std::string& kind) {
  return kind == "config_file" || kind == "flags" || kind == "manifest" ||
         kind == "config_value";
}
}  // namespace

EduDebugLogger& EduDebugLogger::Instance() {
  static EduDebugLogger instance;
  return instance;
}

void EduDebugLogger::Init(const std::string& module,
                          const std::string& channel) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (active_.load()) {
    return;
  }
  module_ = module;
  channel_ = channel.empty() ? ("/apollo/edu_debug/" + module) : channel;
  auto node = apollo::cyber::CreateNode("edu_debug_" + module);
  if (node == nullptr) {
    AERROR << "[EDUDBG] cannot create cyber node for " << module_;
    return;
  }
  writer_ = node->CreateWriter<DebugDump>(channel_);
  if (writer_ == nullptr) {
    AERROR << "[EDUDBG] cannot create writer on " << channel_;
    return;
  }
  active_.store(true);
  AINFO << "[EDUDBG] publishing module debug on " << channel_;
}

std::string EduDebugLogger::ReadFile(const std::string& path) {
  std::ifstream input(path, std::ios::in | std::ios::binary);
  if (!input.is_open()) {
    return std::string();
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

bool EduDebugLogger::FileStat(const std::string& path, std::size_t* size,
                              double* mtime) {
  struct stat info;
  if (::stat(path.c_str(), &info) != 0) {
    return false;
  }
  if (size != nullptr) {
    *size = static_cast<std::size_t>(info.st_size);
  }
  if (mtime != nullptr) {
    *mtime = static_cast<double>(info.st_mtime);
  }
  return true;
}

std::string EduDebugLogger::RealPath(const std::string& path) {
  char resolved[PATH_MAX];
  if (::realpath(path.c_str(), resolved) == nullptr) {
    return path;
  }
  return std::string(resolved);
}

std::string EduDebugLogger::Sha1Of(const std::string& content) {
  Sha1 sha;
  sha.Update(content.data(), content.size());
  return sha.HexDigest();
}

std::string EduDebugLogger::NowIso() {
  const double now = apollo::cyber::Clock::NowInSeconds();
  const std::time_t seconds = static_cast<std::time_t>(now);
  std::tm tm_value;
  ::localtime_r(&seconds, &tm_value);
  char buffer[64];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_value);
  return std::string(buffer);
}

bool EduDebugLogger::ShouldPublish(double min_interval_sec,
                                   const std::string& key) {
  if (!active_.load()) {
    return false;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  const double now = apollo::cyber::Clock::NowInSeconds();
  auto found = last_publish_.find(key);
  if (found != last_publish_.end() &&
      now - found->second < min_interval_sec) {
    return false;
  }
  last_publish_[key] = now;
  return true;
}

void EduDebugLogger::Emit(DebugDump* dump, bool cache) {
  if (!active_.load() || dump == nullptr) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(mutex_);
    dump->set_module(module_);
    dump->set_wall_time(apollo::cyber::Clock::NowInSeconds());
    dump->set_sequence_num(sequence_++);
    if (writer_ != nullptr) {
      writer_->Write(*dump);
    }
    if (cache) {
      if (cache_bytes_ + static_cast<std::size_t>(dump->ByteSizeLong()) <=
          kMaxCacheBytes) {
        cache_.push_back(*dump);
        cache_bytes_ += static_cast<std::size_t>(dump->ByteSizeLong());
      }
    }
  }
  if (dump->kind() == "state" || dump->kind() == "event" ||
      dump->kind() == "config_value") {
    std::string text;
    for (const auto& entry : dump->entries()) {
      if (!text.empty()) {
        text += " ";
      }
      text += entry.key() + "=" + entry.value();
    }
    if (text.size() > 1600) {
      text.resize(1600);
    }
    if (dump->kind() == "event") {
      AINFO << "[EDUDBG][" << dump->source() << "] " << dump->name() << " "
            << text;
    } else if (dump->kind() == "state") {
      ADEBUG << "[EDUDBG][" << dump->source() << "] " << dump->name() << " "
             << text;
    } else {
      AINFO << "[EDUDBG][" << dump->source() << "] " << dump->name() << " "
            << text;
    }
  } else {
    AINFO << "[EDUDBG][" << dump->source() << "] " << dump->kind() << " "
          << dump->name() << " bytes=" << dump->text().size()
          << " chunks=" << dump->chunk_total();
  }
}

void EduDebugLogger::Publish(DebugDump* dump, bool cache) {
  if (!active_.load() || dump == nullptr) {
    return;
  }
  if (dump->text().empty() || dump->text().size() <= kChunkBytes) {
    dump->set_chunk_index(0);
    dump->set_chunk_total(1);
    Emit(dump, cache);
    return;
  }
  const std::size_t total =
      (dump->text().size() + kChunkBytes - 1) / kChunkBytes;
  const std::string full_text = dump->text();
  for (std::size_t index = 0; index < total; ++index) {
    DebugDump chunk;
    chunk.CopyFrom(*dump);
    const std::size_t begin = index * kChunkBytes;
    const std::size_t length =
        std::min(kChunkBytes, full_text.size() - begin);
    chunk.set_text(full_text.substr(begin, length));
    chunk.set_chunk_index(static_cast<std::uint32_t>(index));
    chunk.set_chunk_total(static_cast<std::uint32_t>(total));
    Emit(&chunk, cache);
  }
}

void EduDebugLogger::DumpText(const std::string& source,
                              const std::string& kind,
                              const std::string& name,
                              const std::string& text) {
  if (!active_.load()) {
    return;
  }
  DebugDump dump;
  dump.set_source(source);
  dump.set_kind(kind);
  dump.set_name(name);
  dump.set_text(text);
  Publish(&dump, IsCachedKind(kind));
}

void EduDebugLogger::DumpKv(const std::string& source,
                            const std::string& kind, const std::string& name,
                            const std::vector<Entry>& entries) {
  if (!active_.load()) {
    return;
  }
  DebugDump dump;
  dump.set_source(source);
  dump.set_kind(kind);
  dump.set_name(name);
  for (const auto& item : entries) {
    auto* entry = dump.add_entries();
    entry->set_key(item.first);
    entry->set_value(item.second);
  }
  Publish(&dump, IsCachedKind(kind));
}

void EduDebugLogger::DumpState(const std::string& source,
                               const std::string& name,
                               const std::vector<Entry>& entries) {
  DumpKv(source, "state", name, entries);
}

void EduDebugLogger::DumpEvent(const std::string& source,
                               const std::string& name,
                               const std::string& detail) {
  DebugDump dump;
  dump.set_source(source);
  dump.set_kind("event");
  dump.set_name(name);
  if (!detail.empty()) {
    auto* entry = dump.add_entries();
    entry->set_key("detail");
    entry->set_value(detail);
  }
  Publish(&dump, false);
}

void EduDebugLogger::DumpFile(const std::string& source,
                              const std::string& path,
                              const std::string& kind) {
  if (!active_.load()) {
    return;
  }
  std::size_t size = 0;
  double mtime = 0.0;
  const bool exists = FileStat(path, &size, &mtime);
  const std::string content = exists ? ReadFile(path) : std::string();
  DebugDump dump;
  dump.set_source(source);
  dump.set_kind(kind);
  dump.set_name(path);
  char stamp[64] = {0};
  const std::time_t mtime_seconds = static_cast<std::time_t>(mtime);
  std::tm tm_value;
  ::localtime_r(&mtime_seconds, &tm_value);
  std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tm_value);
  auto add = [&dump](const std::string& key, const std::string& value) {
    auto* entry = dump.add_entries();
    entry->set_key(key);
    entry->set_value(value);
  };
  add("path", path);
  add("realpath", RealPath(path));
  add("exists", exists ? "true" : "false");
  add("size", std::to_string(size));
  add("mtime", exists ? stamp : "");
  add("sha1", exists ? Sha1Of(content) : "");
  dump.set_text(content);
  Publish(&dump, true);
  WatchFile(path);
}

void EduDebugLogger::DumpAllFlags(const std::string& source) {
  if (!active_.load()) {
    return;
  }
  std::vector<google::CommandLineFlagInfo> flags;
  google::GetAllFlags(&flags);
  std::ostringstream out;
  for (const auto& flag : flags) {
    out << flag.name << "=" << flag.current_value;
    if (!flag.is_default) {
      out << "\t#default=" << flag.default_value;
    }
    if (!flag.filename.empty()) {
      out << "\t#file=" << flag.filename;
    }
    out << "\n";
  }
  DumpText(source, "flags", "all_flags", out.str());
}

void EduDebugLogger::DumpTree(const std::string& source,
                              const std::string& root,
                              const std::vector<std::string>& suffixes) {
  if (!active_.load()) {
    return;
  }
  struct Walker {
    explicit Walker(const std::vector<std::string>& suffix_list)
        : suffixes(suffix_list) {}

    const std::vector<std::string>& suffixes;
    std::set<std::string> visited;
    std::ostringstream manifest;
    std::vector<std::string> files;
    int count = 0;

    void Walk(const std::string& dir, int depth) {
      if (depth > 6 || count > 4000) {
        return;
      }
      const std::string real = RealPath(dir);
      if (!visited.insert(real).second) {
        return;
      }
      DIR* handle = ::opendir(dir.c_str());
      if (handle == nullptr) {
        return;
      }
      struct dirent* item = nullptr;
      while ((item = ::readdir(handle)) != nullptr) {
        const std::string name = item->d_name;
        if (name == "." || name == "..") {
          continue;
        }
        const std::string path = dir + "/" + name;
        struct stat info;
        if (::stat(path.c_str(), &info) != 0) {
          continue;
        }
        if (S_ISDIR(info.st_mode)) {
          if (name == "testdata" || name == "test_data") {
            continue;
          }
          Walk(path, depth + 1);
          continue;
        }
        if (!S_ISREG(info.st_mode) || info.st_size > 4 * 1024 * 1024) {
          continue;
        }
        bool matches = suffixes.empty();
        for (const auto& suffix : suffixes) {
          if (EndsWith(path, suffix)) {
            matches = true;
            break;
          }
        }
        if (!matches) {
          continue;
        }
        const std::string content = ReadFile(path);
        char stamp[64] = {0};
        const std::time_t mtime_seconds = info.st_mtime;
        std::tm tm_value;
        ::localtime_r(&mtime_seconds, &tm_value);
        std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tm_value);
        manifest << path << "\t" << RealPath(path) << "\t" << info.st_size
                 << "\t" << stamp << "\t" << Sha1Of(content) << "\n";
        files.push_back(path);
        ++count;
      }
      ::closedir(handle);
    }
  } walker{suffixes};

  walker.Walk(root, 0);
  AINFO << "[EDUDBG][" << source << "] config files under " << root << ": "
        << walker.files.size();
  DumpText(source, "manifest", root, walker.manifest.str());
  for (const auto& path : walker.files) {
    DumpFile(source, path, "config_file");
  }
}

void EduDebugLogger::WatchFile(const std::string& path) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (watched_.find(path) != watched_.end()) {
    return;
  }
  watched_[path] = Sha1Of(ReadFile(path));
  watched_order_.push_back(path);
}

void EduDebugLogger::CheckWatchedFiles() {
  std::vector<Entry> changed;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& path : watched_order_) {
      const std::string current = Sha1Of(ReadFile(path));
      auto found = watched_.find(path);
      if (found != watched_.end() && found->second != current) {
        changed.emplace_back(path, found->second + " -> " + current);
        found->second = current;
      }
    }
  }
  if (changed.empty()) {
    return;
  }
  for (const auto& item : changed) {
    AERROR << "[EDUDBG] config file changed on disk while running: "
           << item.first << " " << item.second;
    DumpEvent("config_watch", "config_file_changed", item.first + " " +
                                                       item.second);
  }
}

void EduDebugLogger::ReplayOne() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (cache_.empty()) {
    return;
  }
  const double now = apollo::cyber::Clock::NowInSeconds();
  if (now < next_replay_time_) {
    return;
  }
  if (replay_index_ >= cache_.size()) {
    replay_index_ = 0;
  }
  DebugDump replay = cache_[replay_index_];
  replay.set_cached(true);
  if (writer_ != nullptr) {
    replay.set_sequence_num(sequence_++);
    writer_->Write(replay);
  }
  ++replay_index_;
  if (replay_index_ >= cache_.size()) {
    replay_index_ = 0;
    next_replay_time_ = now + kReplayCyclePauseSec;
  } else {
    next_replay_time_ = now + kReplayIntervalSec;
  }
}

void EduDebugLogger::Tick() {
  if (!active_.load()) {
    return;
  }
  ReplayOne();
  const double now = apollo::cyber::Clock::NowInSeconds();
  if (now >= next_watch_time_) {
    next_watch_time_ = now + kWatchIntervalSec;
    CheckWatchedFiles();
  }
}

}  // namespace edu_debug
}  // namespace apollo

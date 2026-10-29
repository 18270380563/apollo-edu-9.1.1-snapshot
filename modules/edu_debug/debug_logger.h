/******************************************************************************
 * Shared debug dumper for planning and control.
 *
 * Every dump is published on /apollo/edu_debug/<module> so that
 * `cyber_recorder record -a` stores configuration snapshots and per-cycle
 * internal states next to the ordinary topics. Startup dumps are cached and
 * replayed slowly, so a recording that starts later still contains them.
 *****************************************************************************/

#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "cyber/cyber.h"
#include "modules/edu_debug/proto/debug_dump.pb.h"

namespace apollo {
namespace edu_debug {

using Entry = std::pair<std::string, std::string>;

class EduDebugLogger {
 public:
  static EduDebugLogger& Instance();

  // Safe to call more than once; only the first call creates the writer.
  void Init(const std::string& module, const std::string& channel = "");

  bool active() const { return active_.load(); }
  const std::string& module() const { return module_; }

  // ---- startup / static dumps (cached and replayed) ----
  void DumpAllFlags(const std::string& source);
  void DumpFile(const std::string& source, const std::string& path,
                const std::string& kind = "config_file");
  void DumpText(const std::string& source, const std::string& kind,
                const std::string& name, const std::string& text);
  void DumpKv(const std::string& source, const std::string& kind,
              const std::string& name, const std::vector<Entry>& entries);
  void DumpTree(const std::string& source, const std::string& root,
                const std::vector<std::string>& suffixes);

  // ---- runtime dumps (published immediately, never cached) ----
  void DumpState(const std::string& source, const std::string& name,
                 const std::vector<Entry>& entries);
  void DumpEvent(const std::string& source, const std::string& name,
                 const std::string& detail = "");

  // Time gated publication: true when this key may publish again.
  bool ShouldPublish(double min_interval_sec, const std::string& key);

  // Re-hash a configuration file periodically and report on-disk drift.
  void WatchFile(const std::string& path);

  // Call once per module cycle; replays cached startup dumps and checks
  // watched files.
  void Tick();

 private:
  EduDebugLogger() = default;

  void Publish(DebugDump* dump, bool cache);
  void ReplayOne();
  void CheckWatchedFiles();
  void Emit(DebugDump* dump, bool cache);

  static std::string ReadFile(const std::string& path);
  static bool FileStat(const std::string& path, std::size_t* size,
                       double* mtime);
  static std::string RealPath(const std::string& path);
  static std::string Sha1Of(const std::string& content);
  static std::string NowIso();

  std::mutex mutex_;
  std::string module_;
  std::string channel_;
  std::atomic<bool> active_{false};
  std::shared_ptr<apollo::cyber::Writer<DebugDump>> writer_;

  std::vector<DebugDump> cache_;
  std::size_t cache_bytes_ = 0;
  std::size_t replay_index_ = 0;
  double next_replay_time_ = 0.0;
  double next_watch_time_ = 0.0;
  std::uint32_t sequence_ = 0;

  std::map<std::string, double> last_publish_;
  std::map<std::string, std::string> watched_;
  std::vector<std::string> watched_order_;
};

}  // namespace edu_debug
}  // namespace apollo

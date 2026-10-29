/******************************************************************************
 * Export /apollo/edu_debug/* messages from a cyber record into plain files.
 *
 *   export_debug_record -f record.00000 -o out_dir
 *
 * Output layout:
 *   out_dir/<module>/config/<name>          dumped config file contents
 *   out_dir/<module>/manifest.tsv           path, realpath, size, mtime, sha1
 *   out_dir/<module>/state/<name>.jsonl     one JSON object per state dump
 *   out_dir/<module>/events.jsonl           events (scenario changes, ...)
 *   out_dir/<module>/trace/<name>.txt       text traces (speed curves)
 *   out_dir/<module>/flags.txt              gflag snapshot
 *****************************************************************************/

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "cyber/cyber.h"
#include "cyber/record/record_reader.h"
#include "modules/edu_debug/proto/debug_dump.pb.h"

namespace {

using apollo::cyber::record::RecordMessage;
using apollo::cyber::record::RecordReader;
using apollo::edu_debug::DebugDump;

std::string Sanitize(const std::string& name) {
  std::string out = name;
  for (auto& ch : out) {
    if (ch == '/' || ch == '\\' || ch == ':' || ch == ' ') {
      ch = '_';
    }
  }
  if (out.size() > 180) {
    out = out.substr(out.size() - 180);
  }
  return out;
}

std::string JsonEscape(const std::string& text) {
  std::string out;
  out.reserve(text.size() + 8);
  for (const char ch : text) {
    switch (ch) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out += ch;
    }
  }
  return out;
}

std::string EntriesToJson(const DebugDump& dump) {
  std::ostringstream out;
  out << "{\"t\":" << dump.wall_time() << ",\"seq\":" << dump.sequence_num()
      << ",\"source\":\"" << JsonEscape(dump.source()) << "\",\"name\":\""
      << JsonEscape(dump.name()) << "\"";
  for (const auto& entry : dump.entries()) {
    out << ",\"" << JsonEscape(entry.key()) << "\":\""
        << JsonEscape(entry.value()) << "\"";
  }
  out << "}";
  return out.str();
}

class Exporter {
 public:
  explicit Exporter(const std::string& out_dir) : out_dir_(out_dir) {}

  void Handle(const DebugDump& dump) {
    const std::string module = dump.module().empty() ? "unknown" : dump.module();
    base_ = out_dir_ + "/" + module;
    const std::string& kind = dump.kind();
    const std::string& name = dump.name();

    if (kind == "state") {
      std::ofstream out(base_ + "/state/" + Sanitize(name) + ".jsonl",
                        std::ios::app);
      out << EntriesToJson(dump) << "\n";
      SeenState(name);
      return;
    }
    if (kind == "event") {
      std::ofstream out(base_ + "/events.jsonl", std::ios::app);
      out << EntriesToJson(dump) << "\n";
      return;
    }
    if (kind == "trace") {
      std::ofstream out(base_ + "/trace/" + Sanitize(name) + ".txt",
                        std::ios::app);
      out << "# t=" << dump.wall_time() << "\n" << dump.text();
      if (!dump.text().empty() && dump.text().back() != '\n') {
        out << "\n";
      }
      return;
    }
    if (kind == "flags") {
      WriteOnce(base_ + "/flags.txt", dump.text());
      return;
    }
    if (kind == "manifest") {
      WriteOnce(base_ + "/manifest_tree.tsv", dump.text());
      return;
    }
    if (kind == "config_file") {
      Accumulate(dump);
      WriteConfigMeta(dump);
      return;
    }
    if (kind == "config_value") {
      std::ofstream out(base_ + "/config_values.txt", std::ios::app);
      out << "### " << name << "\n" << dump.text() << "\n";
      return;
    }
  }

 private:
  void SeenState(const std::string& name) {
    if (seen_state_.insert(name).second) {
      AINFO << "[export] state stream " << name;
    }
  }

  void WriteOnce(const std::string& path, const std::string& text) {
    if (written_.count(path) != 0) {
      return;
    }
    written_.insert(path);
    std::ofstream out(path);
    out << text;
  }

  void WriteConfigMeta(const DebugDump& dump) {
    if (!meta_started_) {
      std::ofstream out(base_ + "/manifest.tsv");
      out << "name\tsource\trealpath\tsize\tmtime\tsha1\n";
      meta_started_ = true;
    }
    std::string realpath;
    std::string size;
    std::string mtime;
    std::string sha1;
    for (const auto& entry : dump.entries()) {
      if (entry.key() == "realpath") {
        realpath = entry.value();
      } else if (entry.key() == "size") {
        size = entry.value();
      } else if (entry.key() == "mtime") {
        mtime = entry.value();
      } else if (entry.key() == "sha1") {
        sha1 = entry.value();
      }
    }
    const std::string key = dump.name() + "|" + sha1;
    if (metadata_.count(key) != 0) {
      return;
    }
    metadata_.insert(key);
    std::ofstream out(base_ + "/manifest.tsv", std::ios::app);
    out << dump.name() << "\t" << dump.source() << "\t" << realpath << "\t"
        << size << "\t" << mtime << "\t" << sha1 << "\n";
  }

  // Config files can be split over several chunks; append them in order and
  // write the reassembled file once all chunks arrived.
  void Accumulate(const DebugDump& dump) {
    const std::string key = dump.source() + "|" + dump.name();
    auto& buffer = chunks_[key];
    buffer.text += dump.text();
    buffer.total = dump.chunk_total();
    ++buffer.received;
    if (buffer.total != 0 && buffer.received >= buffer.total) {
      const std::string path = base_ + "/config/" + Sanitize(dump.name());
      std::ofstream out(path);
      out << buffer.text;
      AINFO << "[export] config " << dump.name() << " -> " << path << " ("
            << buffer.text.size() << " B)";
      buffer = ChunkBuffer();
    }
  }

  struct ChunkBuffer {
    std::string text;
    int total = 0;
    int received = 0;
  };

  std::string out_dir_;
  std::string base_;
  bool meta_started_ = false;
  std::map<std::string, ChunkBuffer> chunks_;
  std::set<std::string> metadata_;
  std::set<std::string> written_;
  std::set<std::string> seen_state_;
};

void Usage() {
  std::printf(
      "usage: export_debug_record -f <record-file> [-o <out-dir>] "
      "[-c <channel-prefix>]\n");
}

}  // namespace

int main(int argc, char** argv) {
  std::string record_file;
  std::string out_dir = "./edu_debug_export";
  std::string channel_prefix = "/apollo/edu_debug/";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-f" && i + 1 < argc) {
      record_file = argv[++i];
    } else if (arg == "-o" && i + 1 < argc) {
      out_dir = argv[++i];
    } else if (arg == "-c" && i + 1 < argc) {
      channel_prefix = argv[++i];
    } else if (arg == "-h" || arg == "--help") {
      Usage();
      return 0;
    } else {
      Usage();
      return 1;
    }
  }
  if (record_file.empty()) {
    Usage();
    return 1;
  }

  apollo::cyber::Init("export_debug_record");
  RecordReader reader(record_file);
  if (!reader.IsValid()) {
    std::printf("cannot open record %s\n", record_file.c_str());
    return 1;
  }

  std::map<std::string, std::size_t> channel_counts;
  Exporter exporter(out_dir);
  RecordMessage message;
  std::size_t total = 0;
  while (reader.ReadMessage(&message)) {
    if (message.channel_name.compare(0, channel_prefix.size(),
                                     channel_prefix) != 0) {
      continue;
    }
    DebugDump dump;
    if (!dump.ParseFromString(message.content)) {
      continue;
    }
    ++channel_counts[message.channel_name];
    ++total;
    exporter.Handle(dump);
  }

  std::printf("read %zu edu_debug messages\n", total);
  for (const auto& item : channel_counts) {
    std::printf("  %-40s %zu\n", item.first.c_str(), item.second);
  }
  std::printf("exported to %s\n", out_dir.c_str());
  return 0;
}

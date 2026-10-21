#!/usr/bin/env bash
# Apollo 只读状态监控的一键启动脚本。
# 默认只绑定 127.0.0.1；实车局域网访问必须显式 HOST=0.0.0.0，并自行承担暴露风险。
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-8848}"

if ! command -v python3 >/dev/null 2>&1; then
  echo "错误：找不到 python3。" >&2
  exit 1
fi

# 不在目标目录留下 __pycache__。
export PYTHONDONTWRITEBYTECODE=1

cd "${HERE}"
exec python3 -B monitor_server.py --host "${HOST}" --port "${PORT}" "$@"

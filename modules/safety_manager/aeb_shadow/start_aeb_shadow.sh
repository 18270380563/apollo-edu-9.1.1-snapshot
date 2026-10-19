#!/usr/bin/env bash
set -eo pipefail

source /opt/apollo/neo/setup.sh
exec python3 /apollo_workspace/modules/safety_manager/aeb_shadow/aeb_shadow.py "$@"

#!/bin/bash

ps -ef | grep -E "(mainboard|cyber_|dreamview|data_cleaner|bazel)" | grep -v grep | awk '{print $2}' | xargs kill
sleep 2
ps -ef | grep -E "(mainboard|cyber_|dreamview|data_cleaner|bazel)" | grep -v grep | awk '{print $2}' | xargs kill -9

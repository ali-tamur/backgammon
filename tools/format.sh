#!/bin/bash

find cc/. -type f \( -name "*.cc" -o -name "*.h" \) | xargs clang-format -i --style=file


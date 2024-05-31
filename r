#!/bin/bash

# Stop on errors
# See https://vaneyckt.io/posts/safer_bash_scripts_with_set_euxo_pipefail/
set -Eeuo pipefail

# Sanity check command line options
usage() {
  echo "Usage: $0 (all|fs|test (n)|debug (n)|clean)"
}

# Parse argument.  $1 is the first argument
case $1 in
  "input")
    make clean
    make

    current_time=$(date +"%m-%d_%I:%M_%p")
    output_folder="output/$current_time"
    mkdir -p "$output_folder"
    # log="$output_folder/log.tsv"
    ./solver input.txt > $output_folder/input.out

    ;;
  *)
    usage
    exit 1
    ;;
esac
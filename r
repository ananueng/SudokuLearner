#!/bin/bash

# Stop on errors
# See https://vaneyckt.io/posts/safer_bash_scripts_with_set_euxo_pipefail/
set -Eeuo pipefail

# Sanity check command line options
usage() {
  echo "Usage: $0 (all|fs|test (n)|debug (n)|clean)"
}
if [ $# -ne 1 ]; then
  usage
  exit
fi

make clean
make

current_time=$(date +"%m-%d_%I:%M_%p")
output_folder="output/$current_time"
mkdir -p "$output_folder"
# log="$output_folder/log.tsv"
cp ./$1.txt $output_folder/$1.in
./solver $1.txt > $output_folder/$1.out


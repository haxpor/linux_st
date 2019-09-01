#!/bin/bash

declare -A DEMOS
declare -a DEMOSOR
DEMOS["Unistd.cpp"]="Testing some POSIX operating system level APIs";                DEMOSOR+=("Unistd.cpp")

print_help() {
    echo "Usage: ./build.sh <command> <optional-parameter>"
    echo "List of commands"
    printf "%10s\t-\t%s\n" "build" "Usage: build <.cpp source file>"
    printf "          \t \t%s\n"   "Build a demo program"
    printf "%10s\t-\t%s\n" "list"  "List all demo programs along with"
    printf "          \t \t%s\n"   "corresponding descriptions"
    printf "%10s\t-\t%s\n" "help"  "Show help text"
}

if [ "$#" -lt 1 ]; then
    print_help
    exit 1
fi

if [ "$1" == "build" ]; then
    if [ "$#" -lt 2 ]; then
        echo "Missing .cpp source file"
        echo "Usage: ./make.sh build <.cpp source file>"
        exit 1
    fi

    g++ -g -fno-exceptions -std=c++11 \
        $2 \
        $3  # if there's any additional optional compilation flags, put in quote is adviced

elif [ "$1" == "list" ]; then
    for i in "${!DEMOSOR[@]}"; do
        printf "%30s - %s\n" "${DEMOSOR[$i]}" "${DEMOS[${DEMOSOR[$i]}]}"
    done
    
else
    print_help
    exit 1
fi

#!/bin/bash

## Note：还未验证 ！！！！！！；

# Change to the project root directory
cd "$(dirname "$0")/.." || exit 1
echo "Current directory: $(pwd)"

# Default mode is hardware
MODE="hardware"

# Define usage function
usage() {
    echo
    echo "Usage: $(basename "$0") [options]"
    echo
    echo "Options:"
    echo "  --hardware      Enable hardware mode (default)"
    echo "  --software      Enable software mode"
    echo "  --mode hardware Set hardware mode"
    echo "  --mode software Set software mode"
    echo "  --help, -h      Show this help message"
    echo
    echo "Examples:"
    echo "  $(basename "$0") --hardware"
    echo "  $(basename "$0") --mode hardware"
    echo "  $(basename "$0") --software"
    echo
}

# Process arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --hardware)
            MODE="hardware"
            shift
            ;;
        --software)
            MODE="software"
            shift
            ;;
        --mode)
            if [[ -n "$2" ]]; then
                if [[ "$2" == "hardware" || "$2" == "software" ]]; then
                    MODE="$2"
                    shift 2
                else
                    echo "Error: Invalid mode '$2'"
                    echo "Expected: hardware or software"
                    usage
                    exit 1
                fi
            else
                echo "Error: --mode requires an argument"
                usage
                exit 1
            fi
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "Error: Invalid argument '$1'"
            usage
            exit 1
            ;;
    esac
done

echo "Running in $MODE mode"

# Create output directory
mkdir -p simulator/output

# Build library for hardware mode
if [[ "$MODE" == "hardware" ]]; then
    echo "Building library for hardware mode..."

    # Determine library name based on OS
    if [[ "$(uname -s)" == "Linux" ]]; then
        LIBNAME="libbutton.so"
    else
        LIBNAME="button.dll"
    fi

    if gcc -shared -o simulator/output/$LIBNAME \
              simulator/adapter_layer/button_adapter.c \
              bits_button.c; then
        echo "Library compilation successful: simulator/output/$LIBNAME"
    else
        echo "Library compilation failed! Switching to software mode."
        MODE="software"
    fi
else
    echo "Skipping library compilation for software mode"
fi

# Start simulator with the selected mode
echo "Starting simulator in $MODE mode..."
python simulator/python_simulator/advanced_v2_sim.py --mode "$MODE"

exit $?
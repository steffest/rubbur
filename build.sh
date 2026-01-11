#!/bin/bash
vamos smake
if [ $? -eq 0 ]; then
    echo "Build successful."
    touch run.flag
else
    echo "Build failed."
fi
# ./run.sh

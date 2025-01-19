#!/bin/bash

PACKAGE="react"
ITERATIONS=5
TOTAL_TIME_PM=0
TOTAL_TIME_NPM=0

echo "Comparing download speeds for $PACKAGE"

# Test your package manager
for i in $(seq 1 $ITERATIONS); do
    echo "Run $i: Installing $PACKAGE with custom package manager..."
    START=$(date +%s.%N)
    ./npm_c_manager install $PACKAGE
    END=$(date +%s.%N)
    TIME=$(echo "$END - $START" | bc)
    echo "Custom Package Manager Time: $TIME seconds"
    TOTAL_TIME_PM=$(echo "$TOTAL_TIME_PM + $TIME" | bc)
done

# Test npm
for i in $(seq 1 $ITERATIONS); do
    echo "Run $i: Installing $PACKAGE with npm..."
    START=$(date +%s.%N)
    npm install $PACKAGE --silent
    END=$(date +%s.%N)
    TIME=$(echo "$END - $START" | bc)
    echo "npm Time: $TIME seconds"
    TOTAL_TIME_NPM=$(echo "$TOTAL_TIME_NPM + $TIME" | bc)
    rm -rf node_modules package-lock.json
done

AVG_TIME_PM=$(echo "$TOTAL_TIME_PM / $ITERATIONS" | bc -l)
AVG_TIME_NPM=$(echo "$TOTAL_TIME_NPM / $ITERATIONS" | bc -l)

echo "Average time for Custom Package Manager: $AVG_TIME_PM seconds"
echo "Average time for npm: $AVG_TIME_NPM seconds"

if (( $(echo "$AVG_TIME_PM < $AVG_TIME_NPM" | bc -l) )); then
    echo "Custom Package Manager is faster by $(echo "$AVG_TIME_NPM - $AVG_TIME_PM" | bc) seconds."
else
    echo "npm is faster by $(echo "$AVG_TIME_PM - $AVG_TIME_NPM" | bc) seconds."
fi

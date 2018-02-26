#!/bin/bash

location=$1

xterm -hold -e "${location}/tests/run_test_property.sh ${location}"&
xterm -hold -e "${location}/tests/run_test_property.sh ${location}"&
xterm -hold -e "${location}/tests/run_test_property.sh ${location}"&
xterm -hold -e "${location}/tests/run_test_property.sh ${location}"&

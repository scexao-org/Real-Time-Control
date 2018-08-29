#!/bin/bash




echo "ssh scexao@scexao2 '/home/scexao/bin/devices/analog_output.py $1 $2 $3'"
ssh scexao@scexao2 "/home/scexao/bin/devices/analog_output.py $1 $2 $3"

#!/bin/bash

# FOLDERS
rivet_home="../..";
input="./test_data";
output="./rivet-test";

# FILES
correct="./correct";
latest="./latest";

# COMMANDS - LEAST OUTPUT LEAST STRESS
# VERBOSITY 0
# --binary required for output
command="$rivet_home/rivet_console $input/test-points2.txt $output -H 0 --binary -x 10 -y 10 -f msgpack -V 0";
old_command_point="$rivet_home/rivet_console $rivet_home/data/Test_Point_Clouds/square_data_210pts_codensity.txt $output --binary -x 10 -y 10 -f msgpack -V 0";
command_points="$rivet_home/rivet_console $input/test-points2.txt $output";
command_points_1="$rivet_home/rivet_console $input/test-points2_1.txt $output";
old_command_metric="$rivet_home/rivet_console $rivet_home/data/metric_ex.txt $output --binary -x 10 -y 10"
command_metric="$rivet_home/rivet_console $input/test-metric_ex_3.txt $output --binary -x 10 -y 10"
command_metric_1="$rivet_home/rivet_console $input/test-metric_ex_2.txt $output --binary -x 10 -y 10"
old_command_bifil="$rivet_home/rivet_console $rivet_home/data/Test_Bifiltrations/bifiltration1.txt $output --binary -x 10 -y 10"
command_bifil="$rivet_home/rivet_console $input/test-bifiltration1.txt $output --binary -x 10 -y 10"
old_command_firep="$rivet_home/rivet_console $rivet_home/data/FIReps/firep_ex.txt $output --binary -x 10 -y 10"
command_firep="$rivet_home/rivet_console $input/test-firep_ex.txt $output --binary -x 10 -y 10"

# TESTS

printf "Running RIVET Tests.\n"
printf "Only expected errors will be printed out on the console.\n"
printf "If all tests don't pass, look at diff between the files: correct, latest\n\n"

# rivet test

printf "Test 0 (old points)\n----------------\n\n" > latest
{ 
	$old_command_point
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 1 (new points)\n----------------\n\n" >> latest
{ 
	$command
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# bifil flag

printf "\n\nTest 2 (--bifil abcd)\n----------------\n\n"
printf "\n\nTest 2 (--bifil abcd)\n----------------\n\n" >> latest
{ 
	$command --bifil abcd
} | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 3 (--bifil degree)\n----------------\n\n" >> latest
{ 
	$command --bifil degree
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 4 (--bifil function)\n----------------\n\n" >> latest
{ 
	$command_points_1 --bifil function --binary
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# max-dist flag

printf "\n\nTest 5 (--maxdist 3.5)\n----------------\n\n" >> latest
{ 
	$command --maxdist 3.5
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 6 (--maxdist -5)\n----------------\n\n"
printf "\n\nTest 6 (--maxdist -5)\n----------------\n\n" >> latest
{ 
	$command --maxdist -5
} | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 7 (--maxdist a)\n----------------\n\n"
printf "\n\nTest 7 (--maxdist a)\n----------------\n\n" >> latest
{ 
	$command --maxdist a
} | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 8 (--maxdist)\n----------------\n\n"
printf "\n\nTest 8 (--maxdist)\n----------------\n\n" >> latest
{ 
	$command --maxdist
} | (head -n 5 && tail -n 5) >> latest

# function flag

printf "\n\nTest 9 (points without function)\n----------------\n\n" >> latest
{ 
	$command_points_1 --binary
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# reverse flags

printf "\n\nTest 10 (--yreverse)\n----------------\n\n" >> latest
{ 
	$command --yreverse
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 11 (--xreverse)\n----------------\n\n" >> latest
{ 
	$command --xreverse
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# datatype flag

printf "\n\nTest 12 (--datatype points)\n----------------\n\n" >> latest
{ 
	$command --datatype points_fn
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 13 (--datatype abcd)\n----------------\n\n"
printf "\n\nTest 13 (--datatype abcd)\n----------------\n\n" >> latest
{ 
	$command --datatype abcd
} | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 14 (--datatype 1234)\n----------------\n\n"
printf "\n\nTest 14 (--datatype 1234)\n----------------\n\n" >> latest
{ 
	$command --datatype 1234
} | (head -n 5 && tail -n 5) >> latest

# Homology, xbins, ybins, format, binary, verbosity

printf "\n\nTest 15 (points)\n----------------\n\n" >> latest
{ 
	$command_points -V 4
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 16 (--xbins 5 --ybins 5 -H 1)\n----------------\n\n" >> latest
{ 
	$command_points --xbins 5 --ybins 5 -H 1 --binary -f msgpack
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 17 (-H -2)\n----------------\n\n"
printf "\n\nTest 17 (-H -2)\n----------------\n\n" >> latest
{ 
	$command_points -H -2 --binary -f msgpack
} | (head -n 5 && tail -n 5) >> latest

# Metric

printf "\n\nTest 18 (old metric)\n----------------\n\n" >> latest
{ 
	$old_command_metric
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 19 (new metric)\n----------------\n\n" >> latest
{ 
	$command_metric
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 20 (metric without function)\n----------------\n\n" >> latest
{ 
	$command_metric_1
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# Bifiltration

printf "\n\nTest 21 (old bifiltration)\n----------------\n\n" >> latest
{ 
	$old_command_bifil
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 22 (new bifiltration)\n----------------\n\n" >> latest
{ 
	$command_bifil
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# FIRep

printf "\n\nTest 23 (old firep)\n----------------\n\n" >> latest
{ 
	$old_command_firep
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 24 (new firep)\n----------------\n\n" >> latest
{ 
	$command_firep
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# function flag

printf "\n\nTest 25 (--function abcd)\n----------------\n\n"
printf "\n\nTest 25 (--function abcd)\n----------------\n\n" >> latest
{ 
	$command_points --function abcd
} | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 26 (--function user)\n----------------\n\n" >> latest
{ 
	$command_points --function user --binary
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# ball density

printf "\n\nTest 27 (--function balldensity[])\n----------------\n\n" >> latest
{ 
	$command_points --function balldensity[] --binary
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 28 (--function balldensity)\n----------------\n\n"
printf "\n\nTest 28 (--function balldensity)\n----------------\n\n" >> latest
{ 
	$command_points --function balldensity --binary
} | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 29 (--function balldensity[1])\n----------------\n\n" >> latest
{ 
	$command_metric --function balldensity[1]
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 30 (--function balldensity[1])\n----------------\n\n" >> latest
{ 
	$command_metric_1 --function balldensity[1]
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# gaussian density

printf "\n\nTest 31 (--function gaussian[])\n----------------\n\n" >> latest
{ 
	$command_points --function gaussian[] --binary
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 32 (--function gaussian)\n----------------\n\n"
printf "\n\nTest 32 (--function gaussian)\n----------------\n\n" >> latest
{ 
	$command_points --function gaussian --binary
} | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 33 (--function gaussian[2])\n----------------\n\n" >> latest
{ 
	$command_metric --function gaussian[2]
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 34 (--bifil function --function gaussian[2])\n----------------\n\n" >> latest
{ 
	$command_metric_1 --bifil function --function gaussian[2]
} 2>&1 | (head -n 5 && tail -n 5) >> latest

# eccentricity

printf "\n\nTest 35 (--function eccentricity[])\n----------------\n\n" >> latest
{ 
	$command_points --function eccentricity[] --binary
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 36 (--function eccentricity)\n----------------\n\n"
printf "\n\nTest 36 (--function eccentricity)\n----------------\n\n" >> latest
{ 
	$command_points --function eccentricity --binary
} | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 37 (--function eccentricity[2])\n----------------\n\n" >> latest
{ 
	$command_metric --function eccentricity[2]
} 2>&1 | (head -n 5 && tail -n 5) >> latest

printf "\n\nTest 38 (--bifil function --function eccentricity[2])\n----------------\n\n" >> latest
{ 
	$command_metric_1 --bifil function --function eccentricity[2]
} 2>&1 | (head -n 5 && tail -n 5) >> latest




# ALL TESTS SHOULD BE ADDED BEFORE THIS

# CHECK IF TESTS PASSED

changes=`diff -u $correct $latest`
if [ -z "$changes" ]; then
	printf "\n\nALL TESTS PASSED.\n"
else
	printf "\n\nTESTS FAILED:\n\n$changes \n"
fi
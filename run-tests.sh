#!/usr/bin/env bash

build/rivet_console data/circle_data_60ptx.txt test.rivet -H1 -x10 -y10 -f R0

diff data/ref/circle_data_60pts.txt_H1_10x10.txt test.rivet

build/rivet_console data/circle_data_240pts_inv_density.txt test.rivet -H0 -x5 -y5

diff data/ref/circle_data_240pts_inv_density.txt_H0_5x5.txt test.rivet
diff data/ref/circle_data_240pts_inv_density.txt_H0_5x5.txt test.rivet
diff data/ref/circle_data_240pts_inv_density.txt_H1_10x10.txt test.rivet
diff data/ref/circle_data_240pts_inv_density.txt_H2_5x5.txt test.rivet
diff data/ref/circle_data_400pts_inv_density_H1_10x10.txt test.rivet
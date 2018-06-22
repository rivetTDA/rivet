//
// Created by Bryn Keller on 11/16/17.
//

#ifndef RIVET_CONSOLE_C_API_H
#define RIVET_CONSOLE_C_API_H

#include <cstdlib>

extern "C" {

//typedef struct {
//    unsigned dimensions;
//    double max_distance;
//    double * data;
//    char * param1_name;
//    char * param2_name;
//} PointCloud;
//

typedef struct {
    void* input_params;
    void* template_points;
    void* arrangement;
} Computed;

typedef struct {
    double birth;
    double death;
    unsigned multiplicity;
} Bar;

typedef struct {
    Bar* bars;
    size_t length;
    double angle;
    double offset;
} BarCode;

typedef struct {
    BarCode* barcodes;
    size_t length;
} BarCodesResult;

typedef struct {
    double x_low;
    double y_low;
    double x_high;
    double y_high;
} ArrangementBounds;

struct rivet_comp;
typedef rivet_comp RivetComputation;

RivetComputation* read_rivet_computation(const char* bytes, size_t length);

//Computed* compute_arrangement_from_point_cloud(PointCloud);
BarCodesResult* barcodes_from_computation(RivetComputation* rivet_computation,
    double* offsets,
    double* angles,
    size_t query_length);

ArrangementBounds bounds_from_computation(RivetComputation* rivet_computation);

void free_rivet_computation(RivetComputation* rivet_computation);

void free_barcodes_result(BarCodesResult* result);
}

#endif //RIVET_CONSOLE_C_API_H

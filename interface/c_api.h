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
    char * error;
    size_t error_length;
} BarCodesResult;

typedef struct {
    double x_low;
    double y_low;
    double x_high;
    double y_high;
} ArrangementBounds;

struct rivet_comp;
typedef rivet_comp RivetComputation;

typedef struct {
    RivetComputation* computation;
    char* error;
    size_t error_length;
} RivetComputationResult;

RivetComputationResult read_rivet_computation(const char* bytes, size_t length);

//Computed* compute_arrangement_from_point_cloud(PointCloud);
BarCodesResult barcodes_from_computation(RivetComputation* rivet_computation,
    double* offsets,
    double* angles,
    size_t query_length);

ArrangementBounds bounds_from_computation(RivetComputation* rivet_computation);

void free_rivet_computation_result(RivetComputationResult rivet_computation);

void free_barcodes_result(BarCodesResult result);

typedef struct {
    int64_t nom;
    int64_t denom;
} Ratio;

typedef struct {
    Ratio *x_grades;
    size_t x_length;
    Ratio *y_grades;
    size_t y_length;
} ExactGrades;

typedef struct {
    unsigned x;
    unsigned y;
    unsigned betti_0;
    unsigned betti_1;
    unsigned betti_2;
} StructurePoint;

typedef struct {
    ExactGrades *grades;
    StructurePoint *points;
    size_t length;
} StructurePoints;

StructurePoints * structure_from_computation(RivetComputation* rivet_computation);

void free_structure_points(StructurePoints *points);

}

#endif //RIVET_CONSOLE_C_API_H

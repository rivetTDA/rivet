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
    void * input_params;
    void * template_points;
    void * arrangement;
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

//Computed* compute_arrangement_from_point_cloud(PointCloud);
BarCodesResult* barcodes_from_bytes(const char* bytes,
                                    size_t length,
                                    double* offsets,
                                    double* angles,
                                    size_t query_length
                                    );

void free_barcodes_result(BarCodesResult *result);

}

#endif //RIVET_CONSOLE_C_API_H

//
// Created by Bryn Keller on 11/16/17.
//


#include "c_api.h"
#include <msgpack.hpp>
#include <dcel/dcel.h>
#include <dcel/arrangement_message.h>
#include <computation.h>
#include <api.h>
#include <vector>
#include "dcel/msgpack_adapters.h"
#include "input_parameters.h"


extern "C"
RivetComputation* read_rivet_computation(const char* bytes, size_t length) {
    try {
        std::istringstream buf(std::string(bytes, length));
        auto computation = from_istream(buf);
        return reinterpret_cast<RivetComputation*>(computation.release());
    } catch (std::exception &e) {
        std::cerr << "RIVET error: " << e.what() << std::endl;
        return nullptr;
    }
}

extern "C"
void free_rivet_computation(RivetComputation* computation) {

    delete reinterpret_cast<ComputationResult*>(computation);
}

extern "C"
BarCodesResult* barcodes_from_computation(RivetComputation* rivet_computation,
                                    double* angles,
                                    double* offsets,
                                    size_t query_length
) {
    try {
        ComputationResult* computation = reinterpret_cast<ComputationResult*>(rivet_computation);

        std::vector<std::pair<double, double>> pos;
        for (size_t i = 0; i < query_length; i++) {
            pos.emplace_back(angles[i], offsets[i]);
        }
        auto query_results = query_barcodes(*computation, pos);
        auto barcodes = new BarCode[query_results.size()];
        for (size_t i = 0; i < query_length; i++) {
            auto query_barcode = std::shared_ptr<Barcode>(std::move(query_results[i]));
            barcodes[i].bars = new Bar[query_barcode->size()];
            auto it = query_barcode->begin();
            for (size_t b = 0; b < query_barcode->size(); b++) {
                barcodes[i].bars[b] = Bar{it->birth, it->death, it->multiplicity};
                it++;
            }
            barcodes[i].length = query_barcode->size();
            barcodes[i].angle = angles[i];
            barcodes[i].offset = offsets[i];
        }
        Bounds bounds = compute_bounds(*computation);
        auto result = new BarCodesResult{ barcodes,
                                          query_results.size(),
                                            };
        return result;
    } catch (std::exception &e) {
        std::cerr << "RIVET error: " << e.what() << std::endl;
        return nullptr;
    }
}

extern "C"
ArrangementBounds bounds_from_computation(RivetComputation * rivet_computation) {
    ComputationResult* computation = reinterpret_cast<ComputationResult*>(rivet_computation);
    auto bounds = compute_bounds(*computation);
    return ArrangementBounds {
            bounds.x_low,
            bounds.y_low,
            bounds.x_high,
            bounds.y_high
    };
}

extern "C"
void free_barcodes_result(BarCodesResult *result) {
    for (size_t bc = 0; bc < result->length; bc++) {
        auto bcp = result->barcodes[bc];
        delete[] bcp.bars;
    }
    delete[] result->barcodes;
    delete result;
}



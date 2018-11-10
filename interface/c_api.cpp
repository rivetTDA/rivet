//
// Created by Bryn Keller on 11/16/17.
//

#include "c_api.h"
#include "dcel/msgpack_adapters.h"
#include "input_parameters.h"
#include <api.h>
#include <computation.h>
#include <dcel/arrangement_message.h>
#include <dcel/dcel.h>
#include <msgpack.hpp>
#include <vector>

extern "C" RivetComputation* read_rivet_computation(const char* bytes, size_t length)
{
    try {
        std::istringstream buf(std::string(bytes, length));
        auto computation = from_istream(buf);
        return reinterpret_cast<RivetComputation*>(computation.release());
    } catch (std::exception& e) {
        std::cerr << "RIVET error: " << e.what() << std::endl;
        return nullptr;
    }
}

extern "C" void free_rivet_computation(RivetComputation* computation)
{

    delete reinterpret_cast<ComputationResult*>(computation);
}

extern "C" BarCodesResult* barcodes_from_computation(RivetComputation* rivet_computation,
    double* angles,
    double* offsets,
    size_t query_length)
{
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
                barcodes[i].bars[b] = Bar{ it->birth, it->death, it->multiplicity };
                it++;
            }
            barcodes[i].length = query_barcode->size();
            barcodes[i].angle = angles[i];
            barcodes[i].offset = offsets[i];
        }
        Bounds bounds = compute_bounds(*computation);
        auto result = new BarCodesResult{
            barcodes,
            query_results.size(),
        };
        return result;
    } catch (std::exception& e) {
        std::cerr << "RIVET error: " << e.what() << std::endl;
        return nullptr;
    }
}

extern "C" ArrangementBounds bounds_from_computation(RivetComputation* rivet_computation)
{
    ComputationResult* computation = reinterpret_cast<ComputationResult*>(rivet_computation);
    auto bounds = compute_bounds(*computation);
    return ArrangementBounds{
        bounds.x_low,
        bounds.y_low,
        bounds.x_high,
        bounds.y_high
    };
}

extern "C" void free_barcodes_result(BarCodesResult* result)
{
    for (size_t bc = 0; bc < result->length; bc++) {
        auto bcp = result->barcodes[bc];
        delete[] bcp.bars;
    }
    delete[] result->barcodes;
    delete result;
}


extern "C" StructurePoints * structure_from_computation(RivetComputation* rivet_computation)
{
    ComputationResult* computation = reinterpret_cast<ComputationResult*>(rivet_computation);
    auto x_exact = new Ratio[computation->arrangement->x_exact.size()];
    for (size_t i = 0; i < computation->arrangement->x_exact.size(); i++) {
        auto x = computation->arrangement->x_exact[i];
        x_exact[i].nom = numerator(x).convert_to<int64_t>();
        x_exact[i].denom = denominator(x).convert_to<int64_t>();
    }
    auto y_exact = new Ratio[computation->arrangement->y_exact.size()];
    for (size_t i = 0; i < computation->arrangement->y_exact.size(); i++) {
        auto y = computation->arrangement->y_exact[i];
        y_exact[i].nom = numerator(y).convert_to<int64_t>();
        y_exact[i].denom = denominator(y).convert_to<int64_t>();
    }
    auto grades = new ExactGrades();
    grades->x_grades = x_exact;
    grades->y_grades = y_exact;
    grades->x_length = computation->arrangement->x_exact.size();
    grades->y_length = computation->arrangement->y_exact.size();

    auto points = new StructurePoint[computation->template_points.size()];
    for (size_t i = 0; i < computation->template_points.size(); i++) {
        auto pt = computation->template_points[i];
        points[i].x = pt.x;
        points[i].y = pt.y;
        points[i].betti_0 = pt.zero;
        points[i].betti_1 = pt.one;
        points[i].betti_2 = pt.two;
    }
    auto result = new StructurePoints();
    result->grades = grades;
    result->points = points;
    result->length = computation->template_points.size();
    return result;
}

void free_structure_points(StructurePoints *points) {
    delete[] points->grades->x_grades;
    delete[] points->grades->y_grades;
    delete points->grades;
    delete[] points->points;
    delete points;
}

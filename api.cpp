//
// Created by Bryn Keller on 11/16/17.
//
#include <boost/archive/binary_iarchive.hpp>
#include "dcel/serialization.h"
#include "api.h"

std::unique_ptr<ComputationResult> from_istream(std::istream &file) {
    std::string type;
    std::getline(file, type);
    InputParameters params;
    TemplatePointsMessage templatePointsMessage;
    ArrangementMessage arrangementMessage;
    if (type == "RIVET_1") {
        boost::archive::binary_iarchive archive(file);
        archive >> params;
        archive >> templatePointsMessage;
        archive >> arrangementMessage;
    } else if (type == "RIVET_msgpack") {
        std::string buffer((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

        msgpack::unpacker pac;
        pac.reserve_buffer( buffer.size() );
        std::copy( buffer.begin(), buffer.end(), pac.buffer() );
        pac.buffer_consumed( buffer.size() );

        msgpack::object_handle oh;
        pac.next(oh);
        auto m1 = oh.get();
        m1.convert(params);
        pac.next(oh);
        auto m2 = oh.get();
        m2.convert(templatePointsMessage);
        pac.next(oh);
        auto m3 = oh.get();
        m3.convert(arrangementMessage);

    } else {
        throw std::runtime_error("Expected a precomputed RIVET file");
    }
    return from_messages(templatePointsMessage, arrangementMessage);
}

std::unique_ptr<ComputationResult> from_messages(
        const TemplatePointsMessage &templatePointsMessage,
        const ArrangementMessage &arrangementMessage) {
    std::__1::unique_ptr<ComputationResult> result(new ComputationResult);
    result->arrangement.reset(new Arrangement);
    *(result->arrangement) = arrangementMessage.to_arrangement();
    std::__1::vector<size_t> ex;
    const size_t* shape = templatePointsMessage.homology_dimensions.shape();
    ex.assign(shape, shape + templatePointsMessage.homology_dimensions.num_dimensions());
    result->homology_dimensions.resize(ex);
    result->homology_dimensions = templatePointsMessage.homology_dimensions;
    result->template_points = templatePointsMessage.template_points;
    return result;
}

std::vector<std::unique_ptr<Barcode>> query_barcodes(const ComputationResult &computation,
                                    const std::vector<std::pair<double, double>> &offset_slopes) {

    Grades grades(computation.arrangement->x_exact, computation.arrangement->y_exact);
    std::vector<std::unique_ptr<Barcode>> result;

    for (auto query : offset_slopes) {
        auto angle = query.first;
        auto offset = query.second;
        auto templ = computation.arrangement->get_barcode_template(angle, offset);
        result.push_back(templ.rescale(angle, offset, computation.template_points, grades));
    }
    return result;
}

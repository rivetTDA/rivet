//
// Created by Bryn Keller on 11/16/17.
//
#include "api.h"

std::unique_ptr<ComputationResult> from_istream(std::istream &file) {
    std::string type;
    std::getline(file, type);
    InputParameters params;
    TemplatePointsMessage templatePointsMessage;
    ArrangementMessage arrangementMessage;
    if (type == "RIVET_msgpack") {
        std::string buffer((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

        msgpack::unpacker pac;
        pac.reserve_buffer( buffer.size() );
        std::copy( buffer.begin(), buffer.end(), pac.buffer() );
        pac.buffer_consumed( buffer.size() );

        msgpack::object_handle oh;
        pac.next(oh);
        auto m1 = oh.get();
//        std::cout << "params" << std::endl;
        m1.convert(params);
        pac.next(oh);
        auto m2 = oh.get();
//        std::cout << "points" << std::endl;
//        break_stream stream;
//        std::cout.rdbuf(&stream);
//        std::cerr.rdbuf(&stream);
        m2.convert(templatePointsMessage);
        pac.next(oh);
        auto m3 = oh.get();
//        std::cout << "arrangement message" << std::endl;
        m3.convert(arrangementMessage);

    } else {
        throw std::runtime_error("Expected a RIVET module invariants file");
    }
    return from_messages(templatePointsMessage, arrangementMessage);
}

std::unique_ptr<ComputationResult> from_messages(
        const TemplatePointsMessage &templatePointsMessage,
        const ArrangementMessage &arrangementMessage) {
//    std::cout << "from_messages" << std::endl;
    std::unique_ptr<ComputationResult> result(new ComputationResult);
    result->arrangement.reset(arrangementMessage.to_arrangement());
    //result->arrangement->test_consistency();
    std::vector<size_t> ex;
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

Bounds compute_bounds(const ComputationResult &computation_result) {
    const auto grades = Grades(computation_result.arrangement->x_exact, computation_result.arrangement->y_exact);
    const auto x_low = grades.x.front();
    const auto y_low = grades.y.front();
    const auto x_high = grades.x.back();
    const auto y_high = grades.y.back();
    return Bounds {
            x_low,
            y_low,
            x_high,
            y_high
    };
}

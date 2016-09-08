//
// Created by Bryn Keller on 7/6/16.
//

#ifndef RIVET_CONSOLE_SERIALIZATION_H
#define RIVET_CONSOLE_SERIALIZATION_H

#include <boost/optional.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/utility.hpp>
//#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>

#include "anchor.h"
#include "barcode_template.h"
#include "dcel.h"
#include "math/template_point.h"
#include "math/template_points_matrix.h"
#include "numerics.h"
#include "type_tag.h"
//namespace boost {
//    namespace multiprecision {
//        template<class Archive>
//        void load(Archive &ar, exact &num, const unsigned int version) {
//            std::string rep;
//            ar &rep;
//            num.assign(rep;
//        }
//
//        template<class Archive>
//        void save(Archive &ar, exact const &num , const unsigned int version) {
//            std::stringstream ss;
//            ss & num;
//            ar &ss.str();
//        }
//    }
//}

BOOST_CLASS_EXPORT(BarcodeTemplate)
BOOST_CLASS_EXPORT(BarTemplate)

template <class Archive>
void Vertex::serialize(Archive& ar, const unsigned int version)
{
    ar& incident_edge& x& y;
}

template <class Archive>
void Halfedge::serialize(Archive& ar, const unsigned int version)
{
    ar& origin& twin& next& prev& face& anchor;
}

template <class Archive>
void Face::serialize(Archive& ar, const unsigned int version)
{
    ar& boundary& dbc& visited;
}

template <class Archive>
void Anchor::serialize(Archive& ar, const unsigned int version)
{
    ar& x_coord& y_coord& entry& dual_line& position& above_line& weight;
}

template <class Archive>
void serialize(Archive& ar, TemplatePointsMatrixEntry& x, const unsigned int version)
{
    ar& x.x& x.y& x.index& x.down& x.left& x.low_simplices& x.high_simplices& x.low_count& x.high_count&
        x.low_index& x.high_index;
}

template <class Archive>
void serialize(Archive& ar, Multigrade& m, const unsigned int version)
{
    ar& m.num_cols& m.simplex_index& m.x& m.y;
}

#include <boost/serialization/split_free.hpp>
BOOST_SERIALIZATION_SPLIT_FREE(unsigned_matrix)

namespace boost {

template <class Archive>
void save(Archive& ar, unsigned_matrix const& mat, const unsigned int& version)
{
    assert(mat.num_dimensions() == 2);
    std::vector<unsigned> dims(mat.shape(), mat.shape() + mat.num_dimensions());
    std::vector<unsigned> data(mat.origin(), mat.origin() + mat.num_elements());
    ar& dims& data;
}

template <class Archive>
void load(Archive& ar, unsigned_matrix& mat, const unsigned int& version)
{
    std::vector<unsigned> dims;
    std::vector<unsigned> data;
    ar& dims& data;
    unsigned_matrix::extent_gen extents;
    auto size = extents[dims[0]][dims[1]];
    std::cerr << "Data: ";
    for (auto i = 0; i < data.size(); i++) {
        std::cerr << data[i] << " ";
    }
    std::cerr << std::endl;
    mat.resize(size);
    std::memcpy(mat.origin(), data.data(), data.size() * sizeof(unsigned));
}
}

#endif //RIVET_CONSOLE_SERIALIZATION_H

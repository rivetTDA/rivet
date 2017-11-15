#include <msgpack.hpp>
#include "numerics.h"

#ifndef MSGPACK_ADAPTERS_H
#define MSGPACK_ADAPTERS_H

// User defined class template specialization
namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {

            template<>
            struct as<exact> {
                exact operator()(msgpack::object const& o) const {
                    throw std::runtime_error("not implemented");
//                    if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
//                    if (o.via.array.size != 2) throw msgpack::type_error();
//                    boost::multiprecision::cpp_int num = o.via.array.ptr[0].as<boost::multiprecision::cpp_int>();
//                    boost::multiprecision::cpp_int denom = o.via.array.ptr[1].as<boost::multiprecision::cpp_int>();
//                    return exact(num, denom);
                }
            };
            template<>
            struct pack<exact> {
                template <typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, exact const& mat) const {
                    std::ostringstream oss;
                    oss << mat;
                    std::string s = oss.str();
                    o.pack(s);
                    return o;
                }
            };

            template<>
            struct as<boost::multiprecision::cpp_int> {
                boost::multiprecision::cpp_int operator()(msgpack::object const& o) const {
                    if (o.type != msgpack::type::STR) throw msgpack::type_error();
                    std::string s(o.via.str.ptr, o.via.str.size);
                    return boost::multiprecision::cpp_int(s);
                }
            };
            template<>
            struct convert<boost::multiprecision::cpp_int> {
                msgpack::object const& operator()(msgpack::object const& o, boost::multiprecision::cpp_int &num) const {
                    if (o.type != msgpack::type::STR) throw msgpack::type_error();
                    std::string s(o.via.str.ptr, o.via.str.size);
                    num = boost::multiprecision::cpp_int(s);
                    return o;
                }
            };
            template<>
            struct pack<boost::multiprecision::cpp_int> {
                template <typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, boost::multiprecision::cpp_int const& mat) const {
                    std::string s = mat.str();
                    o.pack_str(s.length());
                    o.pack_str_body(s.data(), s.length());
                    return o;
                }
            };

            template<>
            struct as<unsigned_matrix> {
                unsigned_matrix operator()(msgpack::object const& o) const {
                    if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
                    if (o.via.array.size != 2) throw msgpack::type_error();
                    unsigned_matrix mat;
                    unsigned_matrix::extent_gen extents;
                    std::vector<unsigned> dims = o.via.array.ptr[0].as<std::vector<unsigned >>();
                    std::vector<unsigned> data = o.via.array.ptr[1].as<std::vector<unsigned>>();
                    auto size = extents[dims[0]][dims[1]];
                    std::cerr << std::endl;
                    mat.resize(size);
                    std::memcpy(mat.origin(), data.data(), data.size() * sizeof(unsigned));
                    return mat;
                }
            };

            template<>
            struct convert<unsigned_matrix> {
                msgpack::object const& operator()(msgpack::object const& o, unsigned_matrix& mat) const {
                    if(o.type != type::ARRAY) { throw type_error(); }
                    if(o.via.array.size != 2) { throw type_error(); }
                    unsigned_matrix::extent_gen extents;
                    std::vector<unsigned> dims = o.via.array.ptr[0].convert();
                    std::vector<unsigned> data = o.via.array.ptr[1].convert();
                    auto size = extents[dims[0]][dims[1]];
                    std::cerr << std::endl;
                    mat.resize(size);
                    std::memcpy(mat.origin(), data.data(), data.size() * sizeof(unsigned));
                    return o;
                }
            };

            template<>
            struct pack<unsigned_matrix> {
                template <typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, unsigned_matrix const& mat) const {
                    std::vector<unsigned> dims(mat.shape(), mat.shape() + mat.num_dimensions());
                    std::vector<unsigned> data(mat.origin(), mat.origin() + mat.num_elements());
                    o.pack_array(2);
                    o.pack(dims);
                    o.pack(data);
                    return o;
                }
            };

            template <>
            struct object_with_zone<unsigned_matrix> {
                void operator()(msgpack::object::with_zone& o, unsigned_matrix const& mat) const {
                    o.type = type::ARRAY;
                    o.via.array.size = 2;
                    std::vector<unsigned> dims(mat.shape(), mat.shape() + mat.num_dimensions());
                    std::vector<unsigned> data(mat.origin(), mat.origin() + mat.num_elements());
                    o.via.array.ptr = static_cast<msgpack::object*>(
                            o.zone.allocate_align(sizeof(msgpack::object) * o.via.array.size, MSGPACK_ZONE_ALIGNOF(msgpack::object)));
                    o.via.array.ptr[0] = msgpack::object(dims, o.zone);
                    o.via.array.ptr[1] = msgpack::object(data, o.zone);
                }
            };

        } // namespace adaptor
    } // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
#endif

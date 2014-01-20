/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2008
 */

#ifndef __SIMPLEX_H__
#define __SIMPLEX_H__

#include <vector>
#include <algorithm>
#include <list>
#include <iostream>

#include "utilities/types.h"

#include <boost/compressed_pair.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/serialization/access.hpp>


/**
 * Class: Simplex
 * Basic simplex class. It stores vertices and data of the given types in 
 * a `boost::compressed_pair` (so if data is an Empty class which is the default, 
 * it incurs no space overhead). The class knows how to compute its own <boundary()>. 
 *
 * Parameter:
 *   V -            vertex type
 *   T -            data type
 *
 * \ingroup topology
 */
template<class V, class T = Empty<> >
class Simplex
{
    public:
        /* Typedefs: Public Types
         *
         *    Vertex -              vertex type (template parameter V)
         *    Data -                data type (template parameter T)
         *    Self -
         *    Boundary -            type in which the boundary is stored
         */
        typedef     V                                                               Vertex;
        typedef     T                                                               Data;
        typedef     Simplex<Vertex, Data>                                           Self;
        class BoundaryIterator;

        /* Typedefs: Internal representation
         *
         *    VertexContainer -     internal representation of the vertices
         *    VerticesDataPair -    `compressed_pair` of VertexContainer and Data
         */
        typedef     std::vector<Vertex>                                             VertexContainer;
        typedef     boost::compressed_pair<VertexContainer, Data>                   VerticesDataPair;
        
        /// \name Constructors 
        /// @{
        //
        /// Constructor: Simplex()
        /// Default constructor
        Simplex()                                                                   {}
        /// Constructor: Simplex(Self other)
        /// Copy constructor
        Simplex(const Self& other): 
            vdpair_(other.vertices(), other.data())                                 {}
        /// Constructor: Simplex(Data d)
        /// Initialize simplex with data
        Simplex(const Data& d)                                                      { data() = d; }
        /// Constructor: Simplex(Iterator bg, Iterator end)
        /// Initialize simplex by copying vertices in range [bg, end)
        template<class Iterator>
        Simplex(Iterator bg, Iterator end, const Data& d = Data())                  { join(bg, end); data() = d; }
        /// Constructor: Simplex(VertexContainer v)
        /// Initialize simplex by copying the given VertexContainer
        Simplex(const VertexContainer& v, const Data& d = Data()):  
            vdpair_(v, d)                                                           { std::sort(vertices().begin(), vertices().end()); }
        /// @}
        
        /// \name Core 
        /// @{
        ///
        /// Functions: boundary_begin(), boundary_end()
        /// Returns the iterators over the boundary of the simplex
        BoundaryIterator        boundary_begin() const;
        BoundaryIterator        boundary_end() const;
        /// Function: dimension()
        /// Returns the dimension of the simplex
        Dimension               dimension() const                                   { return vertices().size() - 1; }
        /// @}
        
        const Data&             data() const                                        { return vdpair_.second(); }
        Data&                   data()                                              { return vdpair_.second(); }
        const VertexContainer&  vertices() const                                    { return vdpair_.first(); }
        
        /// \name Vertex manipulation
        /// @{
        bool                    contains(const Vertex& v) const;
        bool                    contains(const Self& s) const;
        void                    add(const Vertex& v);
        template<class Iterator>
        void                    join(Iterator bg, Iterator end);
        void                    join(const Self& other)                             { join(other.vertices().begin(), other.vertices().end()); }
        /// @}

        const Self&             operator=(const Self& s)                            { vdpair_ = s.vdpair_; return *this; }

        std::ostream&           operator<<(std::ostream& out) const;

        /* Classes: Comparisons
         *
         * VertexComparison -           compare simplices based on the lexicographic ordering of their <vertices()>
         * VertexDimensionComparison -  compare simplices based on the lexicographic ordering of their <vertices()> within each dimension
         * DataComparison -             compare simplices based on their <data()>
         * DataDimensionComparison -    compare simplices based on their <data()> within each <dimension()>
         */
        struct VertexComparison;
        struct VertexDimensionComparison;
        struct DataComparison;
        struct DataDimensionComparison;

        /* Classes: Functors
         *
         * DataEvaluator -              return data given a simplex
         * DimensionExtractor -         return dimesnion given a simplex
         */
        struct DataEvaluator;
        struct DimensionExtractor;
    
    protected:
        VertexContainer&        vertices()                                          { return vdpair_.first(); }

        VerticesDataPair        vdpair_;

    protected:
        /* Serialization */
        friend class boost::serialization::access;
        
        template<class Archive>
        void                    serialize(Archive& ar, version_type );
};


template<class V, class T>
struct Simplex<V,T>::VertexComparison
{
        typedef                 Self                    first_argument_type;
        typedef                 Self                    second_argument_type;
        typedef                 bool                    result_type;

        bool                    operator()(const Self& a, const Self& b) const       { return a.vertices() < b.vertices(); }
};

template<class V, class T>
struct Simplex<V,T>::VertexDimensionComparison
{
        typedef                 Self                    first_argument_type;
        typedef                 Self                    second_argument_type;
        typedef                 bool                    result_type;

        bool                    operator()(const Self& a, const Self& b) const       
        { 
            if (a.dimension() == b.dimension())
                return a.vertices() < b.vertices();
            else
                return a.dimension() < b.dimension();
        }
};

template<class V, class T>
struct Simplex<V,T>::DataComparison
{
        typedef                 Self                    first_argument_type;
        typedef                 Self                    second_argument_type;
        typedef                 bool                    result_type;

        bool                    operator()(const Self& a, const Self& b) const       { return a.data() < b.data(); }
};

template<class S>
struct DataDimensionComparison
{
        typedef                 S                       Simplex;
        typedef                 Simplex                 first_argument_type;
        typedef                 Simplex                 second_argument_type;
        typedef                 bool                    result_type;

        bool                    operator()(const Simplex& a, const Simplex& b) const       
        {
            if (a.dimension() == b.dimension())
                return a.data() < b.data();
            else
                return a.dimension() < b.dimension();
        }
};
        
template<class V, class T>
struct Simplex<V,T>::DataEvaluator
{
        typedef                 Self                    first_argument_type;
        typedef                 Data                    result_type;

        result_type             operator()(const first_argument_type& s) const      { return s.data(); }
};

template<class V, class T>
struct Simplex<V,T>::DimensionExtractor
{
        typedef                 Self                    first_argument_type;
        typedef                 Dimension               result_type;

        result_type             operator()(const first_argument_type& s) const      { return s.dimension(); }
};


// TODO: class DirectSimplex - class which stores indices of the simplices in its boundary
// TODO: class CompactSimplex<V, T, N> - uses arrays instead of vectors to store simplices 
//       (dimension N must be known at compile time)


#include "simplex.hpp"

#endif // __SIMPLEX_H__

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/nvp.hpp>

#include <boost/iterator/filter_iterator.hpp>
#include <functional>

/* Implementations */

template<class V, class T>
struct Simplex<V,T>::BoundaryIterator: public boost::iterator_adaptor<BoundaryIterator,                                 // Derived
                                                                      typename VertexContainer::const_iterator,         // Base
                                                                      Simplex<V,T>,                                     // Value
                                                                      boost::use_default,
                                                                      Simplex<V,T> >
{
    public:
        typedef     typename VertexContainer::const_iterator                Iterator;
        typedef     boost::iterator_adaptor<BoundaryIterator,
                                            Iterator,
                                            Simplex<V,T>,
                                            boost::use_default,
                                            Simplex<V,T> >                  Parent;

                    BoundaryIterator()                                      {}
        explicit    BoundaryIterator(Iterator iter, const VertexContainer& vertices):
                        Parent(iter), vertices_(vertices)                   {}

    private:
        friend class    boost::iterator_core_access;
        Simplex<V,T>    dereference() const
        {
            typedef     std::not_equal_to<Vertex>                           NotEqualVertex;

            return      Self(boost::make_filter_iterator(std::bind2nd(NotEqualVertex(), *(this->base())), vertices_.begin(), vertices_.end()),
                             boost::make_filter_iterator(std::bind2nd(NotEqualVertex(), *(this->base())), vertices_.end(),   vertices_.end()));
        }

        const VertexContainer&      vertices_;
};

/* Simplex */
template<class V, class T>
typename Simplex<V,T>::BoundaryIterator
Simplex<V,T>::
boundary_begin() const
{
    if (dimension() == 0)   return boundary_end();
    return BoundaryIterator(vertices().begin(), vertices());
}

template<class V, class T>
typename Simplex<V,T>::BoundaryIterator
Simplex<V,T>::
boundary_end() const
{
    return BoundaryIterator(vertices().end(), vertices());
}

template<class V, class T>
bool
Simplex<V,T>::
contains(const Vertex& v) const
{
    // TODO: would std::find() be faster? (since most simplices we deal with are low dimensional)
    typename VertexContainer::const_iterator location = std::lower_bound(vertices().begin(), vertices().end(), v);
    return ((location != vertices().end()) && (*location == v));
}

template<class V, class T>
bool
Simplex<V,T>::
contains(const Self& s) const
{
    return std::includes(  vertices().begin(),   vertices().end(),
                         s.vertices().begin(), s.vertices().end());
}

template<class V, class T>
void
Simplex<V,T>::
add(const Vertex& v)
{
    // TODO: would find() or lower_bound() followed by insert be faster?
    vertices().push_back(v); std::sort(vertices().begin(), vertices().end());
}

template<class V, class T>
template<class Iterator>
void
Simplex<V,T>::
join(Iterator bg, Iterator end)
{
    vertices().insert(vertices().end(), bg, end);
    std::sort(vertices().begin(), vertices().end());
}

template<class V, class T>
std::ostream&
Simplex<V,T>::
operator<<(std::ostream& out) const
{
    typename VertexContainer::const_iterator cur = vertices().begin();
    out << "<" << *cur;
    for (++cur; cur != vertices().end(); ++cur)
    {
        out << ", " << *cur;
    }
    out << ">";
    // out << " [" << data() << "] ";

    return out;
}

template<class V, class T>
template<class Archive>
void
Simplex<V,T>::
serialize(Archive& ar, version_type )
{
    ar & boost::serialization::make_nvp("vertices", vertices());
    ar & boost::serialization::make_nvp("data", data());
}

template<class V, class T>
std::ostream& operator<<(std::ostream& out, const Simplex<V,T>& s)
{ return s.operator<<(out); }

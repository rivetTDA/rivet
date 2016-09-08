//
// Created by Bryn Keller on 7/14/16.
//

#ifndef RIVET_CONSOLE_TYPE_TAG_H
#define RIVET_CONSOLE_TYPE_TAG_H

//http://www.ilikebigbits.com/blog/2014/5/6/type-safe-identifiers-in-c
template<class Tag, class impl, impl default_value>
class ID
{
public:
    static ID invalid() { return ID(); }

    // Defaults to ID::invalid()
    ID() : m_val(default_value) { }

    // Explicit constructor:
    explicit ID(impl val) : m_val(val) { }

    // Explicit conversion to get back the impl:
    explicit operator impl() const { return m_val; }

    friend bool operator==(ID a, ID b) { return a.m_val == b.m_val; }
    friend bool operator!=(ID a, ID b) { return a.m_val != b.m_val; }

    template<class Archive>
            void serialize(Archive & ar, unsigned const int & version) {
        ar & m_val;
    }
private:
    impl m_val;
};


#endif //RIVET_CONSOLE_TYPE_TAG_H

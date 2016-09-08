#ifndef RIVET_DEBUG_H
#define RIVET_DEBUG_H

#include <iostream>
#include <memory>

#ifdef QT_CORE_LIB

#include <QDebug>
using Debug = QDebug;

Debug debug(bool nospace = false);

#else

struct Debug {
    std::ostream& os;
    bool space;
    bool alive;

    Debug(std::ostream& os = std::clog, bool space = true)
        : os(os)
        , space(space)
        , alive(true)
    {
    }
    Debug(Debug&& rhs)
        : os(rhs.os)
        , space(rhs.space)
        , alive(true)
    {
        rhs.alive = false;
    }
    Debug(Debug& rhs)
        : os(rhs.os)
        , space(rhs.space)
        , alive(true)
    {
        rhs.alive = false;
    }
    ~Debug()
    {
        if (alive && space)
            os << std::endl;
    }
};

template <typename T>
Debug operator<<(Debug&& d, T const& x)
{
    (d.os) << x;
    if (d.space)
        d.os << " ";
    return std::move(d);
}

template <typename T>
Debug operator<<(Debug& d, T const& x)
{
    d.os << x;
    if (d.space)
        d.os << " ";
    return std::move(d);
}

Debug debug(bool nospace = false, std::ostream& out = std::clog);

#endif
#endif

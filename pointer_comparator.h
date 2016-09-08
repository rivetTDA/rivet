//
// Created by Bryn Keller on 6/29/16.
//

#ifndef RIVET_CONSOLE_POINTER_COMPARATOR_H
#define RIVET_CONSOLE_POINTER_COMPARATOR_H

template <typename T, typename Comparator>
class PointerComparator {

public:
    Comparator comparator;

    bool operator()(const std::shared_ptr<T> lhs, const std::shared_ptr<T> rhs) const //returns true if lhs comes before rhs
    {
        return comparator(*lhs, *rhs);
    }
};
#endif //RIVET_CONSOLE_POINTER_COMPARATOR_H

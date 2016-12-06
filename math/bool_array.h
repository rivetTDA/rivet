//
// Created by Bryn Keller on 11/22/16.
//

#ifndef RIVET_CONSOLE_BOOL_ARRAY_H
#define RIVET_CONSOLE_BOOL_ARRAY_H

#include <memory>
//Helper class to manage dynamically sized 2d arrays of bools. Not a serious array class, just a helper for
//a specific use case.
class bool_array {
public:
    bool_array(unsigned long rows, unsigned long cols);
    bool & at(unsigned long row, unsigned long col);

private:
    unsigned long cols;
    std::unique_ptr<bool[]> array;
};


#endif //RIVET_CONSOLE_BOOL_ARRAY_H

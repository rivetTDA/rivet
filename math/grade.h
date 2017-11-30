//
//  grade.h
//  
//
//  Created by mlesnick on 11/19/17.
//
//

#ifndef grade_h
#define grade_h

/*
Note: This file contains a Struct that appeared earlier in bifiltration_data.h.  Since the same struct is
 useful elsewhere, it is better in its own file.
*/

//Pair of coordinates specifying grade of appearance with additional sorting operator. Sorted COLEXICOGRAPHICALLY, i.e., first by y-coordinate,  then by x-coordinate.
struct Grade {
    int x;
    int y;
    
    bool operator==(const Grade& other) const
    {
        return x == other.x && y == other.y;
    }
    
    bool operator<(const Grade& other) const
    {
        if (y != other.y)
            return y < other.y;
        else
            return x < other.x;
    }
    
    Grade() {}
    
    Grade(int set_x, int set_y)
    : x(set_x)
    , y(set_y)
    {
    }
};


#endif /* grade_h */

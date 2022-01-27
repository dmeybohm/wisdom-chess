#ifndef WISDOM_COORD_ITERATOR_HPP
#define WISDOM_COORD_ITERATOR_HPP

#include "global.hpp"
#include "coord.hpp"

#define FOR_EACH_ROW_AND_COL(row, col) \
   for (row = 0; row < wisdom::Num_Rows; row++) \
       for (col = 0; col < wisdom::Num_Columns; col++)


#endif //WISDOM_COORD_ITERATOR_HPP

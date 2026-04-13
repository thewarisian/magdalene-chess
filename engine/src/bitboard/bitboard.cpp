#include <iostream>

#include "bitboard/bitboard.h"

namespace bitboard {
    void display(bitmap b, char occupiedSymbol, char emptySymbol) {
            for (int i = chessmeta::NUM_TILES - 1; i >= 0; i--) {
            Square sq = utils::intToSquare(i);

            //Display symbol of occupancy
            std::cout << (occupiedAt(b, sq) ? occupiedSymbol : emptySymbol);

            if (i % 8 == 0) {
                std::cout << "\n";
            }   
        }
    }    
}
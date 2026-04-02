#include <iostream>

#include "bitboard/bitboard.h"

namespace bitboard {
    void display(bitmap b, char occupiedSymbol, char emptySymbol) {
        for(int i = chessmeta::NUM_TILES-1; i >= 0; i--) {
            // Print symbol based on occupancy
            std::cout << (occupiedAt(b, i) ? occupiedSymbol : emptySymbol);
            if(i % 8 == 0) { std::cout << "\n"; }
        }
    }
}
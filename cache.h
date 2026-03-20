#include <cstdint>
using bitmap = uint64_t;

// Initial position in FEN notation
const char* INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// File bitboards (columns)
const bitmap FILE_BITS[8] = {
    0x0101010101010101ULL, // FILE_A
    0x0202020202020202ULL, // FILE_B
    0x0404040404040404ULL, // FILE_C
    0x0808080808080808ULL, // FILE_D
    0x1010101010101010ULL, // FILE_E
    0x2020202020202020ULL, // FILE_F
    0x4040404040404040ULL, // FILE_G
    0x8080808080808080ULL  // FILE_H
};

// Rank bitboards (rows)
const bitmap RANK_BITS[8] = {
    0x00000000000000FFULL, // RANK_1
    0x000000000000FF00ULL, // RANK_2
    0x0000000000FF0000ULL, // RANK_3
    0x00000000FF000000ULL, // RANK_4
    0x000000FF00000000ULL, // RANK_5
    0x0000FF0000000000ULL, // RANK_6
    0x00FF000000000000ULL, // RANK_7
    0xFF00000000000000ULL  // RANK_8
};

// Precomputed attack bitboards for each piece type and square
const bitmap PAWN_ATTACKS[2][64] = {}; // Pawn attacks for white (0) and black (1)
const bitmap KNIGHT_ATTACKS[64] = {};  // Knight attacks from each square
const bitmap BISHOP_ATTACKS[64] = {};  // Bishop attacks from each square
const bitmap ROOK_ATTACKS[64] = {};    // Rook attacks from each square
const bitmap QUEEN_ATTACKS[64] = {};   // Queen attacks from each square
const bitmap KING_ATTACKS[64] = {};    // King attacks from each square
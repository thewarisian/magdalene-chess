#include <string>
#include "cache.h"

/**
 * @class ChessBoard
 * @brief Represents a chessboard using bitboards.
 *
 * This class stores the state of a chess game using 64-bit bitmaps
 * for each piece type and color. It supports initialization via
 * FEN (Forsyth-Edwards Notation).
 */
class ChessBoard {
private:
    // ===================== PIECE BITBOARDS =====================

    /// Bitboard representing all white pawns
    bitmap whitePawns;
    /// Bitboard representing all white knights
    bitmap whiteKnights;
    /// Bitboard representing all white bishops
    bitmap whiteBishops;
    /// Bitboard representing all white rooks
    bitmap whiteRooks;
    /// Bitboard representing all white queens
    bitmap whiteQueens;
    /// Bitboard representing the white king
    bitmap whiteKing;

    /// Bitboard representing all black pawns
    bitmap blackPawns;
    /// Bitboard representing all black knights
    bitmap blackKnights;
    /// Bitboard representing all black bishops
    bitmap blackBishops;
    /// Bitboard representing all black rooks
    bitmap blackRooks;
    /// Bitboard representing all black queens
    bitmap blackQueens;
    /// Bitboard representing the black king
    bitmap blackKing;

    // ===================== INTERNAL METHODS =====================

    /**
     * @brief Initializes the board state from a FEN string.
     *
     * Parses the FEN string and assigns piece positions
     * to the corresponding bitboards.
     *
     * @param fen A valid FEN string describing a chess position.
     */
    void initializeBoard(const std::string& fen);

public:
    // ===================== CONSTRUCTORS =====================

    /**
     * @brief Constructs a ChessBoard from a FEN string.
     *
     * If no FEN string is provided, the board is initialized
     * to the standard starting position.
     *
     * @param fen FEN string (default = initial chess position).
     */
    ChessBoard(const std::string& fen = INITIAL_FEN);

    // ===================== PUBLIC METHODS =====================

    /**
     * @brief Returns a human-readable representation of the board.
     *
     * Typically used for debugging or console output.
     *
     * @return A string representing the board layout.
     */
    std::string display();
};
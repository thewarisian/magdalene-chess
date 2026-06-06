/**
 * @file movegen.h
 * @brief Pseudo-legal move generation using bitboard operations.
 *
 * This file contains functions responsible for generating pseudo-legal
 * moves from a given board state. Move generation is performed using
 * efficient bitwise operations on bitboards.
 *
 * Scope:
 * - Generates destination squares only (bitboards)
 * - Does NOT enforce move legality (e.g., king safety, pins)
 *
 * @note
 * - All functions assume a consistent bitboard mapping and correctly
 *   initialized GameBoard state.
 */
#pragma once

#include <vector>

#include "board/chessboard.h"
#include "bitboard/bitboard.h"

using bb = bitboard::bitmap;

/**
 * @namespace movegen
 * @brief Contains functions responsible for generating pseudo-legal moves.
 *
 * This namespace operates on raw bitboards passed by the caller rather than
 * querying board state internally, minimising redundant copies during search.
 *
 * Design Overview:
 * - Sliding pieces (rook, bishop, queen) use the Hyperbola Quintessence algorithm
 * - Leaping pieces (knight, king) use precomputed O(1) lookup tables
 * - Pawns are handled separately due to directional asymmetry
 * - Queens are computed as the union of rook and bishop mobility
 *
 * All functions generate pseudo-legal moves only. Legality filtering
 * (pins, checks, castling rights) is handled at a higher level.
 */
namespace movegen {

    /**
     * @brief Converts a move bitboard into a list of destination squares.
     *
     * Iterates over all set bits and converts each square index into
     * algebraic notation (e.g., "e4").
     *
     * @param movesBitboard Bitboard with set bits representing destination squares
     * @return Vector of destination squares as strings
     *
     * @note
     * - Uses LSB extraction for efficient iteration
     * - Order is determined by bit index (low → high), not board order
     *
     * @warning Intended for debugging and visualization only
     *
     * @complexity O(k), where k = number of set bits
     */
    std::vector<std::string> getMovesList(bb movesBitboard);

    /**
     * @brief Generates pseudo-legal moves for all pawns of a given color.
     *
     * Includes single pushes, double pushes from starting rank, diagonal captures,
     * and en passant (caller must include en passant square in enemyCapturables).
     *
     * @param col              Color of the pawns
     * @param pawns            Bitboard of pawns to move
     * @param enemyCapturables Bitboard of capturable squares (enemy pieces + en passant)
     * @param emptySquares     Bitboard of unoccupied squares
     * @return Bitboard of all reachable destination squares
     *
     * @note Does NOT handle promotions. Pseudo-legal only — no pin or check handling.
     */
    bb calculatePawnMoves(Color col, bb pawns, bb enemyCapturables, bb emptySquares);

    /**
     * @brief Generates all squares diagonally attacked by pawns of a given color.
     *
     * Returns all squares a pawn attacks regardless of occupancy — used for
     * attack map generation and king safety checks. Unlike calculatePawnMoves,
     * does not require an enemy piece or en passant square to be present.
     *
     * @param col   Color of the pawns
     * @param pawns Bitboard of pawns
     * @return Bitboard of all diagonally attacked squares
     *
     * @note Excludes pawn pushes — diagonal attacks only
     * @see calculatePawnMoves, calculatePlayerAttacks
     */
    bb calculatePawnAttacks(Color col, bb pawns);

    /**
     * @brief Computes sliding piece attacks along a single ray using the Hyperbola Quintessence algorithm.
     *
     * Generates all attacked squares for a sliding piece on a given ray (rank, file,
     * diagonal, or anti-diagonal) in both directions simultaneously, correctly
     * stopping at blockers and including enemy captures.
     *
     * Algorithm:
     * Given occupancy o, slider s, and ray mask m:
     *   attacks = ((o&m) - 2s) ^ reverse(reverse(o&m) - 2*reverse(s))) & m
     *
     * - Forward subtraction propagates attacks toward higher-index squares
     * - Reversed subtraction propagates attacks toward lower-index squares
     * - XOR combines both directions
     * - Masking with m restricts results to the ray
     * - Masking with ~friendOccupied removes friendly piece destinations
     *
     * @param sq             Square the sliding piece occupies
     * @param occupied       Bitboard of all occupied squares
     * @param friendOccupied Bitboard of squares occupied by friendly pieces
     * @param visionMask     Ray mask (file, rank, diagonal, or anti-diagonal)
     * @return Bitboard of all reachable squares along the ray
     *
     * @note
     * - Call once per ray axis; combine results for full piece mobility
     * - Enemy pieces are included as captures; friendly pieces block and are excluded
     *
     * @complexity O(1) — fully branchless bitboard arithmetic
     * @see reverseBitmap, calculateRookTypeMoves, calculateBishopTypeMoves
     */
    bb hypbQuint(Square sq, bb occupied, bb friendOccupied, bb visionMask);

    /**
     * @brief Generates pseudo-legal rook moves from a given square.
     *
     * Applies Hyperbola Quintessence along the file and rank axes
     * and unions the results.
     *
     * @param occupied       Bitboard of all occupied squares
     * @param friendOccupied Bitboard of friendly occupied squares
     * @param sq             Square the rook occupies
     * @return Bitboard of all reachable squares
     *
     * @see hypbQuint
     */
    bb calculateRookTypeMoves(bb occupied, bb friendOccupied, Square sq);

    /**
     * @brief Generates pseudo-legal bishop moves from a given square.
     *
     * Applies Hyperbola Quintessence along the diagonal and anti-diagonal axes
     * and unions the results.
     *
     * @param occupied       Bitboard of all occupied squares
     * @param friendOccupied Bitboard of friendly occupied squares
     * @param sq             Square the bishop occupies
     * @return Bitboard of all reachable squares
     *
     * @see hypbQuint, getDiagonalMask, getAntiDiagonalMask
     */
    bb calculateBishopTypeMoves(bb occupied, bb friendOccupied, Square sq);

    /**
     * @brief Generates pseudo-legal knight moves from a given square.
     *
     * Performs a single precomputed table lookup and masks out friendly pieces.
     *
     * @param friendOccupied Bitboard of friendly occupied squares
     * @param sq             Square the knight occupies
     * @return Bitboard of all reachable squares
     *
     * @note O(1). Knights jump over all pieces.
     */
    bb calculateKnightMoves(bb friendOccupied, Square sq);

    /**
     * @brief Generates pseudo-legal king moves from a given square.
     *
     * Performs a single precomputed table lookup and masks out friendly pieces.
     *
     * @param friendOccupied Bitboard of friendly occupied squares
     * @param sq             Square the king occupies
     * @return Bitboard of all reachable squares
     *
     * @note
     * - O(1) table lookup
     * - Does NOT handle castling or moving into check
     */
    bb calculateKingMoves(bb friendOccupied, Square sq);

    /**
     * @brief Dispatches move generation for a single piece by type.
     *
     * Routes to the appropriate move generation function based on piece type.
     *
     * @param occupied       Bitboard of all occupied squares
     * @param friendOccupied Bitboard of friendly occupied squares
     * @param sq             Square the piece occupies
     * @param pt             Type of the piece
     * @return Bitboard of all reachable squares
     *
     * @warning Do NOT use for pawns — pawn moves require color and additional
     *          context not available here. Pawn input returns 0.
     */
    bb calculateTypeMoves(bb occupied, bb friendOccupied, Square sq, PieceType pt);

    /**
     * @brief Generates combined pseudo-legal moves for all pieces of a given type.
     *
     * Iterates over each piece in the bitboard, computes its moves via
     * calculateTypeMoves, and unions the results.
     *
     * @param pieces         Bitboard of all pieces of the given type to move
     * @param occupied       Bitboard of all occupied squares
     * @param friendOccupied Bitboard of friendly occupied squares
     * @param pt             Piece type (must not be PAWN)
     * @return Union bitboard of all reachable squares across all pieces
     *
     * @complexity O(k), where k = number of pieces
     */
    bb calculateMajorPieceMovesOfType(bb pieces, bb occupied, bb friendOccupied, PieceType pt);
    
    /**
     * @brief Generates all squares attacked by a player.
     *
     * Computes the union of all squares attacked by every piece of the given color.
     * Used primarily for king safety — squares in this set are unsafe for the enemy king.
     *
     * Pawn attacks are computed via calculatePawnAttacks (all diagonal squares regardless
     * of occupancy), not calculatePawnMoves, so empty squares covered by pawns are
     * correctly marked as dangerous.
     *
     * @param col            Color of the attacking player
     * @param pawns          Pawn bitboard
     * @param knights        Knight bitboard
     * @param bishops        Bishop bitboard
     * @param rooks          Rook bitboard
     * @param queens         Queen bitboard
     * @param king           King bitboard
     * @param occupied       Bitboard of all occupied squares
     * @param friendOccupied Bitboard of the attacking player's own pieces
     * @return Union bitboard of all attacked squares
     *
     * @note Does not include pawn push squares — only diagonal pawn attacks
     */
    bitboard::bitmap calculatePlayerAttacks(Color col,
                                            bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king,
                                            bb occupied, bb friendOccupied);

    /**
     * @brief Generates legal moves for a player, accounting for king safety.
     *
     * Computes pseudo-legal moves for all pieces then filters king moves to
     * exclude squares attacked by the enemy. The enemy attack map is computed
     * with the player's king removed from occupancy to prevent the king from
     * hiding behind itself against sliding pieces.
     *
     * @param col          Color of the player to move
     * @param enemyEnPassant En passant target square bitboard (0 if none)
     * @param pawns        Player pawn bitboard
     * @param knights      Player knight bitboard
     * @param bishops      Player bishop bitboard
     * @param rooks        Player rook bitboard
     * @param queens       Player queen bitboard
     * @param king         Player king bitboard
     * @param enemyPawns   Enemy pawn bitboard
     * @param enemyKnights Enemy knight bitboard
     * @param enemyBishops Enemy bishop bitboard
     * @param enemyRooks   Enemy rook bitboard
     * @param enemyQueens  Enemy queen bitboard
     * @param enemyKing    Enemy king bitboard
     * @param empty        Bitboard of empty squares
     * @param occupied     Bitboard of all occupied squares
     * @param friendOccupied Bitboard of player's own pieces
     * @param enemyOccupied  Bitboard of enemy pieces
     * @return Bitboard of all legal destination squares
     *
     * @note Does NOT yet handle: pins, check evasion, castling
     */
    bitboard::bitmap calculateLegalPlayerMoves(Color col, bb enemyEnPassant,
                                            bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king,
                                            bb enemyPawns, bb enemyKnights, bb enemyBishops, bb enemyRooks, bb enemyQueens, bb enemyKing,
                                            bb empty, bb occupied, bb friendOccupied, bb enemyOccupied);
}
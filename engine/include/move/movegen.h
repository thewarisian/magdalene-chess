/**
 * @file movegen.h
 * @brief High-performance move and attack mask generation using bitboard operations.
 *
 * This file contains the foundational interfaces for tracking tactical terrain,
 * calculating sliding/hopping piece attack vectors via Hyperbola Quintessence, 
 * and generating advanced filtering masks (such as Check Masks) to resolve 
 * absolute legal move constraints.
 *
 * Scope:
 * - Generates raw pseudo-legal bitboards for individual pieces or piece groups.
 * - Establishes absolute check evasion masks (capture/block runways) for the army.
 * - Isolates empty sliding rays between disparate squares for pinning and checking lines.
 *
 * @note All functions assume a consistent bitboard mapping (e.g., LSB-to-MSB) 
 * and an appropriately initialized game state.
 */
#pragma once

#include <vector>
#include <string>

#include "bitboard/bitboard.h" 
#include "board/chessboard.h"
#include "core/utils.h"

namespace movegen {

    // Using the same bitboard type alias defined in your .cpp file
    using bb = bitboard::bitmap;

    /**
     * @brief Converts a bitboard of target destinations into a list of coordinate strings.
     *
     * Iteratively extracts individual set bits from the provided bitboard using 
     * Least Significant Bit (LSB) isolation, translating each index into its 
     * human-readable algebraic notation equivalent (e.g., "e4", "g5").
     *
     * @param movesBitboard A bitboard containing the target destination bits.
     * @return A std::vector of std::string objects representing the valid squares.
     *
     * @note Destructive to the local copy of the bitboard via bit popping.
     */
    std::vector<std::string> getMovesList(bb movesBitboard);

    /**
     * @brief Calculates all pseudo-legal destinations for a collection of pawns.
     *
     * Computes color-specific pawn mechanics including single-step forward pushes, 
     * double-step pushes (accounting for intermediate square collisions), and 
     * diagonal attacks matching against hostile targets or en passant flags.
     *
     * @param col The Color of the pawns currently moving.
     * @param pawns A bitboard containing the locations of moving pawns.
     * @param enemyCapturables A bitboard representing pieces available for capture (or EP targets).
     * @param emptySquares A bitboard representing un-occupied squares on the board (~occupied).
     * @return A bitboard of all valid pawn destination squares.
     */
    bb calculatePawnMoves(Color col, bb pawns, bb enemyCapturables, bb emptySquares);

    /**
     * @brief Generates an attack map representing raw pawn diagonal threats.
     *
     * Unlike movement calculation, this isolates strictly where pawns are casting
     * attacks/defenses, ignoring forward pushes and ignoring whether an enemy 
     * piece is actually present on the target diagonals.
     *
     * @param col The Color of the attacking pawns.
     * @param pawns A bitboard containing the locations of the pawns.
     * @return A combined bitboard map of all targeted squares.
     */
    bb calculatePawnAttacks(Color col, bb pawns);

    /**
     * @brief Core sliding attack engine utilizing Hyperbola Quintessence subtraction math.
     *
     * Employs the optimized binary calculation `((masked - 2s) ^ ((masked)' - 2s')')` 
     * to simultaneously compute higher-side and lower-side sliding attacks along a 
     * pre-allocated alignment track.
     *
     * @param slider A single-bit bitboard representing the position of the sliding piece.
     * @param occupied A bitboard showing all blocking configurations currently on the board.
     * @param visionMask A precomputed bitboard track (Rank, File, Diagonal) the slider is bound to.
     * @return A bitboard map of attacks running up to and including the first blocking obstacles.
     */
    bb hypbQuint(bb slider, bb occupied, bb visionMask);

    /**
     * @brief Computes orthogonal sliding attacks for a single rook square.
     *
     * Combines distinct file and rank Hyperbola Quintessence queries centered on the piece.
     *
     * @param rook A single-bit bitboard containing the rook's location.
     * @param occupied The global occupancy bitboard.
     * @return A bitboard containing all valid orthogonal attack vectors.
     */
    bb calculateRookAttacks(bb rook, bb occupied);

    /**
     * @brief Computes diagonal sliding attacks for a single bishop square.
     *
     * Combines distinct diagonal and anti-diagonal Hyperbola Quintessence queries centered on the piece.
     *
     * @param bishop A single-bit bitboard containing the bishop's location.
     * @param occupied The global occupancy bitboard.
     * @return A bitboard containing all valid diagonal attack vectors.
     */
    bb calculateBishopAttacks(bb bishop, bb occupied);

    /**
     * @brief Computes omnidirectional sliding attacks for a single queen square.
     *
     * Intersects rook-based linear tracks and bishop-based diagonal tracks.
     *
     * @param queen A single-bit bitboard containing the queen's location.
     * @param occupied The global occupancy bitboard.
     * @return A combined bitboard containing all 8 sliding attack lines.
     */
    bb calculateQueenAttacks(bb queen, bb occupied);

    /**
     * @brief Fetches jumping attack destinations for a single knight square.
     *
     * @param knight A single-bit bitboard containing the knight's location.
     * @return A pre-calculated O(1) bitboard lookup of valid knight hops.
     */
    bb calculateKnightAttacks(bb knight);

    /**
     * @brief Fetches adjacent step destinations for a single king square.
     *
     * @param king A single-bit bitboard containing the king's location.
     * @return A pre-calculated O(1) bitboard lookup of adjacent king steps.
     * @note Does not evaluate checking vulnerabilities, check protection, or castling legality.
     */
    bb calculateKingAttacks(bb king);

    /**
     * @brief Evaluates attacks for a single piece based dynamically on its type.
     *
     * Acts as an internal routing hub forwarding calculation variables to individual
     * sliding or leaping handlers matching the specified PieceType enum.
     *
     * @param piece A single-bit bitboard containing the piece's location.
     * @param occupied The global occupancy bitboard.
     * @param pt The explicit PieceType descriptor of the piece.
     * @return A target bitboard representing that piece's attacks.
     */
    bb calculateAttacksOfType(bb piece, bb occupied, PieceType pt);

    /**
     * @brief Loops over grouped pieces to map their unified pseudo-legal attack footprints.
     *
     * Desorbs an entire bitboard of pieces one-by-one using LSB isolation loops, 
     * summarizing their distinct ranges into an aggregated total map.
     *
     * @param pieces A bitboard group containing all active pieces of the targeted type.
     * @param occupied The global occupancy bitboard.
     * @param pt The explicit PieceType descriptor of the group.
     * @return A unified bitboard aggregating all attacks generated by the input group.
     */
    bb calculateAllPieceMovesOfType(bb pieces, bb occupied, PieceType pt);
    
    /**
     * @brief Assembles a master pseudo-legal attack/control grid for an entire color's faction.
     *
     * Evaluates pawns, knights, rooks, bishops, queens, and the king in sequence, returning
     * a massive comprehensive snapshot of every single square currently watched, protected, 
     * or targeted by the faction.
     *
     * @param col The Color of the active player army being mapped.
     * @return A comprehensive map of targeted squares across the entire chessboard.
     */
    bb calculatePseudoLegalMoves(Color col, bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king, bb occupied);

    /**
     * @brief Isolates the exclusive empty pathway stretching directly between two aligned squares.
     *
     * Employs an optimized two-way symmetric Hyperbola Quintessence intersection layout. By running
     * rays from both endpoints toward each other and intersecting them via bitwise AND, 
     * the endpoints are naturally cleared out, leaving exactly the empty tracking track between them.
     *
     * @param from The starting point square (typically the King's square).
     * @param to The endpoint square (typically an attacking or pinning enemy slider).
     * @return A bitboard highlighting strictly the empty lane squares bridging from and to.
     * If the squares do not align along a file, rank, or diagonal, returns 0ULL.
     */
    bb calculateRay(bb from, bb to);

    /**
     * @brief Computes the absolute legal restriction mask for standard pieces during check conditions.
     *
     * Employs reverse-piece symmetry outward from the king's square to isolate exactly what enemy 
     * threats are actively dealing a check. Evaluates the check severity state:
     * - **Zero Checks:** Returns all ones (~0ULL); the army has normal tactical freedom.
     * - **Double/Multi Check:** Returns zero (0ULL); non-king pieces are completely paralyzed 
     * as blocking or capturing cannot resolve multiple vectors simultaneously.
     * - **Single Check:** Computes a strict mask containing the attacker's target square (for capture)
     * combined with the intermediate sliding track (for blocking intercepts).
     *
     * @param col The color of the friendly player defending against checks.
     * @param king A single-bit bitboard marking the king's current position.
     * @param enemyPawns Bitboard of enemy pawns.
     * @param enemyKnights Bitboard of enemy knights.
     * @param enemyBishops Bitboard of enemy bishops.
     * @param enemyRooks Bitboard of enemy rooks.
     * @param enemyQueens Bitboard of enemy queens.
     * @param occupied The global occupancy configuration.
     * @return A strict filtering mask. Non-king moves must bitwise-AND with this to ensure move legality.
     */
    bb calculateCheckMask(Color col, bb king, bb enemyPawns, bb enemyKnights, bb enemyBishops, bb enemyRooks, bb enemyQueens, bb occupied);

    /**
     * @brief Generates an absolute pin restriction mask for every square on the board.
     *
     * This function detects absolute pins against the King using a two-pass bitwise 
     * X-ray technique. It simulates sliding rays outward from the King's square to 
     * find friendly pieces backed up against threatening enemy sliders.
     * * The process is executed in two optimized stages:
     * 1. **Orthogonal Sweep:** Checks for pins along ranks and files caused by enemy Rooks or Queens.
     * 2. **Diagonal Sweep:** Checks for pins along diagonals and anti-diagonals caused by enemy Bishops or Queens.
     * * Pinned candidates are temporarily removed from a mock occupancy board (`xRay`) using bitwise XOR,
     * allowing the King's visual path to pass through them and register the hiding enemy attacker.
     * Verified pinning bitboards are merged together via a bitwise OR assignment (`|=`), leading 
     * to a single, highly performant bit-popping resolution loop.
     *
     * @param col The Color of the friendly side defending against potential pins.
     * @param occupied The global occupancy configuration of the entire board.
     * @param pawns Bitboard of friendly pawns.
     * @param knights Bitboard of friendly knights.
     * @param bishops Bitboard of friendly bishops.
     * @param rooks Bitboard of friendly rooks.
     * @param queens Bitboard of friendly queens.
     * @param king A single-bit bitboard marking the friendly King's position.
     * @param enemyPawns Bitboard of enemy pawns.
     * @param enemyKnights Bitboard of enemy knights.
     * @param enemyBishops Bitboard of enemy bishops.
     * @param enemyRooks Bitboard of enemy rooks.
     * @param enemyQueens Bitboard of enemy queens.
     * * @return A std::vector<bb> of size 64 (`chessmeta::NUM_TILES`). 
     * - Unpinned piece squares retain a value of all ones (`~0ULL`), denoting full freedom.
     * - Pinned piece squares are overwritten with a strict mask containing the empty sliding rail 
     * bridging the King and the attacker, bitwise-OR'ed with the attacker's own square bit.
     */
    std::vector<bb> calculatePinMasks(Color col, bb occupied,
        bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king, 
        bb enemyPawns, bb enemyKnights, bb enemyBishops, bb enemyRooks, bb enemyQueens);

} // namespace movegen
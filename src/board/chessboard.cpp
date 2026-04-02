#include "board/chessboard.h"

namespace chessboard {
    void GameBoard::updateBitboards() {
        //All white pieces on board
        whitePieces = 
            whitePawns | whiteKnights | whiteBishops |
            whiteRooks | whiteQueens  | whiteKing;

        //All black pieces on board
        blackPieces = 
            blackPawns | blackKnights | blackBishops |
            blackRooks | blackQueens  | blackKing;

        //Squares with some piece
        occupiedSquares = whitePieces | blackPieces;
        //Squares with no piece
        emptySquares = ~occupiedSquares;
    }

    void GameBoard::updateBoardAfterMove(chessmove::Move m) {
        //Update en passant
        if(chessmove::isDoublePawnPush(m)) { 
            if(m.attackPieceType == 'P') { enPassantIdx = m.toBitIdx - 8; }
            else if(m.attackPieceType == 'p') { enPassantIdx = m.toBitIdx + 8; }
        }
        //Since en passant can only be made immediately after double push
        else { enPassantIdx = -1; }

        //Reverse player;
        whiteToMove = !whiteToMove;
        //Change all bitboards
        updateBitboards();
        //TODO add other updates
    }

    // ===================== INTERNAL METHODS (board initialisation / manipulation) =====================

    int GameBoard::placePiece(char ch, int bitIdx) {
        switch(ch) {
            // ===== WHITE PIECES =====
            case 'P': bitboard::placeBitAt(whitePawns, bitIdx); break;
            case 'N': bitboard::placeBitAt(whiteKnights, bitIdx); break;
            case 'B': bitboard::placeBitAt(whiteBishops, bitIdx); break;
            case 'R': bitboard::placeBitAt(whiteRooks, bitIdx); break;
            case 'Q': bitboard::placeBitAt(whiteQueens, bitIdx); break;
            case 'K': bitboard::placeBitAt(whiteKing, bitIdx); break;

            // ===== BLACK PIECES =====
            case 'p': bitboard::placeBitAt(blackPawns, bitIdx); break;
            case 'n': bitboard::placeBitAt(blackKnights, bitIdx); break;
            case 'b': bitboard::placeBitAt(blackBishops, bitIdx); break;
            case 'r': bitboard::placeBitAt(blackRooks, bitIdx); break;
            case 'q': bitboard::placeBitAt(blackQueens, bitIdx); break;
            case 'k': bitboard::placeBitAt(blackKing, bitIdx); break;

            // ===== EMPTY SQUARES =====
            // Digit represents number of consecutive empty squares
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8':
                return (ch - '0');

            // ===== RANK SEPARATOR =====
            // Ignored due to linear bottom-up indexing
            case '/':
                return 0;

            // ===== FALLBACK =====
            // Unknown character → advance one square (safe default)
            default:
                return 1;
        }

        // All piece placements advance by 1 square
        return 1;
    }

    void GameBoard::removePiece(char ch, int bitIdx) {
        switch(ch) {
            // White pieces
            case 'P': bitboard::removeBitAt(whitePawns, bitIdx); break;
            case 'N': bitboard::removeBitAt(whiteKnights, bitIdx); break;
            case 'B': bitboard::removeBitAt(whiteBishops, bitIdx); break;
            case 'R': bitboard::removeBitAt(whiteRooks, bitIdx); break;
            case 'Q': bitboard::removeBitAt(whiteQueens, bitIdx); break;
            case 'K': bitboard::removeBitAt(whiteKing, bitIdx); break;

            // Black pieces
            case 'p': bitboard::removeBitAt(blackPawns, bitIdx); break;
            case 'n': bitboard::removeBitAt(blackKnights, bitIdx); break;
            case 'b': bitboard::removeBitAt(blackBishops, bitIdx); break;
            case 'r': bitboard::removeBitAt(blackRooks, bitIdx); break;
            case 'q': bitboard::removeBitAt(blackQueens, bitIdx); break;
            case 'k': bitboard::removeBitAt(blackKing, bitIdx); break;

            //Empty target square
            case 'E' : 
            // unknown char (could warn)
            default: break;
        }
    }

    void GameBoard::setBoard(const std::string& reducedFen) {
        int pos = 0;
        for(int j = reducedFen.size() - 1; j >= 0; j--) {
            const char& ch = reducedFen[j];
            pos += placePiece(ch, pos);
        }
    }

    void GameBoard::applyFenString(const std::string& fen) {
        std::istringstream iss(fen);
        std::string reducedFen, toMove, castling, enpass;

        if(!(iss >> reducedFen >> toMove >> castling >> enpass >> halfMove >> fullMove)) {
            throw std::invalid_argument("Malformed FEN");
        }

        // Set piece positions
        setBoard(reducedFen);

        // Player to move
        whiteToMove = toMove == "w";

        // Castling rights
        whiteCanCastleKingSide = whiteCanCastleQueenSide = false;
        blackCanCastleKingSide = blackCanCastleQueenSide = false;
        for(char ch : castling) {
            switch(ch) {
                case 'K': whiteCanCastleKingSide = true; break;
                case 'Q': whiteCanCastleQueenSide = true; break;
                case 'k': blackCanCastleKingSide = true; break;
                case 'q': blackCanCastleQueenSide = true; break;
            }
        }

        // En passant target
        enPassantIdx = enpass == "-" ? -1 : tileStringToBitIndex(enpass);
    }

    void GameBoard::applyBoardMatrix(const std::array<std::array<char, 8>, 8>& boardMatrix) {
        //Places pieces from matrix onto board
        for(int i = chessmeta::NUM_ROWS-1; i >= 0; i--) {
            for(int j = chessmeta::NUM_COLS-1; j >= 0; j--) {
                int bitIdx = (chessmeta::NUM_TILES-1) - (8*i + j);
                bitIdx += placePiece(boardMatrix[i][j], bitIdx);
            }
        }
    }

    // ===================== CONSTRUCTORS =====================

    GameBoard::GameBoard(const std::string& fen) {
        // Reset all bitboards
        whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0;
        blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0;

        applyFenString(fen);
        updateBitboards();
    }

    GameBoard::GameBoard(const std::array<std::array<char, 8>, 8> boardMatrix, bool wToMove, 
    bool wCanCastKing, bool wCanCastQueen, bool bCanCastKing, bool bCanCastQueen,
    int enPassIdx, int numHalf, int numFull) {
        //Setting other parameters
        whiteToMove = wToMove;
        whiteCanCastleKingSide = wCanCastKing;
        whiteCanCastleQueenSide = wCanCastQueen;
        blackCanCastleKingSide = bCanCastKing;
        blackCanCastleQueenSide = bCanCastQueen;
        enPassantIdx = enPassIdx;
        halfMove = numHalf;
        fullMove = numFull;

        // Reset all bitboards
        whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0;
        blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0;
        applyBoardMatrix(boardMatrix);

        updateBitboards();
    }

    // ===================== PUBLIC GETTER METHODS =====================

    std::string GameBoard::toString() const {
        std::string rep = "";
        for(int i = 0; i < chessmeta::NUM_TILES; i++) {
            if(i != 0 && i % 8 == 0) rep = "\n\n" + rep;

            if(bitboard::occupiedAt(whitePawns, i))        { rep = "P  " + rep; }
            else if(bitboard::occupiedAt(whiteKnights, i)) { rep = "N  " + rep; }
            else if(bitboard::occupiedAt(whiteBishops, i)) { rep = "B  " + rep; }
            else if(bitboard::occupiedAt(whiteRooks, i))   { rep = "R  " + rep; }
            else if(bitboard::occupiedAt(whiteQueens, i))  { rep = "Q  " + rep; }
            else if(bitboard::occupiedAt(whiteKing, i))    { rep = "K  " + rep; }
            else if(bitboard::occupiedAt(blackPawns, i))   { rep = "p  " + rep; }
            else if(bitboard::occupiedAt(blackKnights,i))  { rep = "n  " + rep; }
            else if(bitboard::occupiedAt(blackBishops,i))  { rep = "b  " + rep; }
            else if(bitboard::occupiedAt(blackRooks, i))   { rep = "r  " + rep; }
            else if(bitboard::occupiedAt(blackQueens,i))   { rep = "q  " + rep; }
            else if(bitboard::occupiedAt(blackKing,i))     { rep = "k  " + rep; }
            
            else { rep = "_  " + rep; }
        }
        return "\n" + rep + "\n";
    }

    bitboard::bitmap GameBoard::getPieceBitboard(char type) const {
        switch(type) {
            // White pieces
            case 'P': return whitePawns;
            case 'N': return whiteKnights;
            case 'B': return whiteBishops;
            case 'R': return whiteRooks;
            case 'Q': return whiteQueens;
            case 'K': return whiteKing;

            // Black pieces
            case 'p': return blackPawns;
            case 'n': return blackKnights;
            case 'b': return blackBishops;
            case 'r': return blackRooks;
            case 'q': return blackQueens;
            case 'k': return blackKing;

            default: throw std::invalid_argument("Invalid piece type");
        }
    }

    bitboard::bitmap GameBoard::getAllPiecesBitboard(char colourFilter) const {
        switch(colourFilter) {
            case 'W': return whitePieces;
            case 'B': return blackPieces;
            case 'A': return occupiedSquares;
            case 'N': return emptySquares;

            default: throw std::invalid_argument("Invalid piece type");
        }
    }

    // ========================= MUTATOR METHODS =======================

    void GameBoard::makeMove(chessmove::Move& move) {
        //TODO: Handle Promotions, En Passant, Castling
        removePiece(move.capturedPieceType, move.toBitIdx);
        removePiece(move.attackPieceType, move.fromBitIdx);
        placePiece(move.attackPieceType, move.toBitIdx);

        updateBoardAfterMove(move);
    }
}

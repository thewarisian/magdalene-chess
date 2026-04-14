# ======== DEFINE STATE =============
layout = [
['r','_','b','q','k','_','_','r'],
['p','p','_','_','_','p','p','p'],
['_','_','n','_','_','_','_','_'],
['_','_','_','p','_','_','_','_'],
['_','_','_','P','_','_','_','_'],
['_','_','N','_','_','_','_','_'],
['P','P','_','_','_','P','P','P'],
['R','_','B','Q','K','_','_','R'],
]

white_to_move = False

white_can_castle_king_side = False
white_can_castle_queen_side = False
black_can_castle_king_side = False
black_can_castle_queen_side = False

en_passant_square = 'e5' #e.g. e4

half_move_counter = 6
full_move_counter = 7

def get_fen(board, white_move, 
            white_castle_king, white_castle_queen, 
            black_castle_king, black_castle_queen, 
            enpass_square, half_count, full_count) -> str:
    """
    @brief Converts the current board state and game metadata into a FEN string.

    Generates a complete Forsyth–Edwards Notation (FEN) representation of the
    given chess position, including piece placement, active color, castling rights,
    en passant target square, and move counters.

    @param board 2D list (8x8) representing the board state.
                 Each element is a character:
                 - 'P','N','B','R','Q','K' for White pieces
                 - 'p','n','b','r','q','k' for Black pieces
                 - '_' for empty squares

    @param white_move Boolean indicating if it is White's turn to move
    @param white_castle_king Boolean indicating White can castle kingside (K)
    @param white_castle_queen Boolean indicating White can castle queenside (Q)
    @param black_castle_king Boolean indicating Black can castle kingside (k)
    @param black_castle_queen Boolean indicating Black can castle queenside (q)
    @param enpass_square Algebraic notation (e.g., 'e3') of en passant target square,
                         or None if no en passant is available
    @param half_count Halfmove clock (for the fifty-move rule)
    @param full_count Fullmove number (starts at 1 and increments after Black’s move)

    @return A string representing the full FEN of the position

    @details
    FEN Format:
    1. Piece placement (8 ranks, '/' separated, top to bottom)
       - Empty squares are compressed into digits
    2. Active color ('w' or 'b')
    3. Castling availability ('KQkq' or '-' if none)
    4. En passant target square ('-' if none)
    5. Halfmove clock
    6. Fullmove number

    Example:
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

    @note
    - Assumes board[0] corresponds to rank 8 and board[7] to rank 1
    - Does not validate legality of the position
    - En passant square must already be computed externally

    @complexity
    O(64) — iterates through all squares once
    """
    fen = ""

    #Piece placement calculation
    for row in board:
        empty = 0
        for ch in row:
            #If empty, add to count
            if ch == '_': empty+=1
            #Previous tiles have been empty, but encountered occupied
            elif empty != 0: 
                fen += str(empty) + ch
                empty = 0
            else: fen += ch
        if empty != 0: fen += str(empty)
        #Add row separator
        fen += '/'

    # remove last extra separator
    fen = fen[:-1]

    #player to move calculation
    fen += (' w' if white_move else ' b')

    #Castling rights calculation
    rights = ""
    if(white_castle_king): rights += 'K'
    if(white_castle_queen): rights += 'Q'
    if(black_castle_king): rights += 'k'
    if(black_castle_queen): rights += 'q'
    #If no castling rights found
    if(rights == ""): rights = '-'
    fen += ' '+rights+' '

    #En passant square addition and counters addition
    fen += ('-' if enpass_square is None else enpass_square)
    fen += f" {half_count} {full_count}"
    
    #Exclude last 
    return fen

current_fen = get_fen(layout, white_to_move, white_can_castle_king_side, white_can_castle_queen_side,
              black_can_castle_king_side, black_can_castle_queen_side, en_passant_square, 
              half_move_counter, full_move_counter)
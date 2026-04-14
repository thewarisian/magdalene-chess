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
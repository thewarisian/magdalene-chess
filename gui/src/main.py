import pygame

import config, board, state

pygame.init()

#Make Window
screen = pygame.display.set_mode(config.WINDOW_DIM)
pygame.display.set_caption(config.WINDOW_TITLE)

#Set fonts
font = pygame.font.SysFont(config.MARGIN_FONT_STYLE, config.MARGIN_FONT_SIZE)

#Variables for drag and drop
dragging = False
dragged_piece = None
drag_pos = None
source_pos = None

#GAME LOOP
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT: running = False

        elif event.type == pygame.MOUSEBUTTONDOWN:
            #Drag and Drop start
            if event.button == 1:
                x_pos, y_pos = event.pos

                #Click made within chessboard
                if config.BOARD_BEGIN <= x_pos <= config.BOARD_END and config.BOARD_BEGIN <= y_pos <= config.BOARD_END:
                    row, col = board.get_board_indices(x_pos, y_pos)

                    if(state.layout[row][col] != '_'): #Piece on selected tile
                        dragging = True
                        dragged_piece = state.layout[row][col]
                        board.remove_piece(state.layout, row, col)
                        source_pos = board.get_board_indices(x_pos, y_pos)

                    #Redraw dragged sprite - set initial position of mouse
                    d_x, d_y = event.pos
                    #Center mouse on sprite
                    d_x -= config.PIECE_SPRITE_SIZE/2
                    d_y -= config.PIECE_SPRITE_SIZE/2
                    drag_pos = d_x, d_y
        
        elif event.type == pygame.MOUSEMOTION:
            #Drag and drop motion
            if dragging:
                d_x, d_y = event.pos
                #Center mouse on sprite
                d_x -= config.PIECE_SPRITE_SIZE/2
                d_y -= config.PIECE_SPRITE_SIZE/2
                drag_pos = d_x, d_y

        elif event.type == pygame.MOUSEBUTTONUP:
            if dragging:
                x_pos, y_pos = event.pos
                
                if config.BOARD_BEGIN <= x_pos <= config.BOARD_END and config.BOARD_BEGIN <= y_pos <= config.BOARD_END:
                    #Place piece in the center of the tile where the mouse was clicked
                    row, col = board.get_board_indices(x_pos, y_pos)

                    #TODO Add sounds for capture, castling, check, invalid
                    if (row, col) != source_pos:
                        config.MOVE_SOUND.play()
                else:
                    #Place back at original tile if dragged outside board
                    row, col = source_pos
                
                #Update board state
                state.layout[row][col] = dragged_piece

                #Reset dragging variables
                dragging = False
                dragged_piece = None
                drag_pos = None
                source_pos = None

    #Draw board
    board.draw_board(screen, font)
    board.place_pieces(screen, state.layout)

    if dragging: board.draw_piece(screen, dragged_piece, drag_pos)
    
    #Display drawn items
    pygame.display.flip()

pygame.quit()
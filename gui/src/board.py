import pygame

import config

def write_text(surface, font, color, text, coords):
    """
    @brief Renders and draws centered text onto a surface.

    Converts the given string into a rendered text surface using the specified
    font and color, then draws it onto the target surface such that the text
    is centered at the provided coordinates.

    @param surface Target Pygame surface to draw onto
    @param font Pygame font object used for rendering text
    @param color RGB tuple specifying text color
    @param text String to be rendered
    @param coords Tuple (x, y) representing the center position for the text

    @note
    - Uses anti-aliased rendering for smoother text appearance.
    - Internally aligns text using a rectangle centered at `coords`.

    @complexity O(1) per call (render + blit operations)
    """
    text_surface = font.render(text, True, color)
    text_rect = text_surface.get_rect(center=coords)
    surface.blit(text_surface, text_rect)

def get_tile_center_coord(row:int, col:int):
    return (
        config.MARGIN_WIDTH + config.TILE_SIZE*col, #x_pos
        config.MARGIN_WIDTH + config.TILE_SIZE*row, #y_pos
    )

def get_board_indices(x_pos:int, y_pos:int):
    return (
        (y_pos - config.MARGIN_WIDTH) // config.TILE_SIZE, #col
        (x_pos - config.MARGIN_WIDTH) // config.TILE_SIZE #row
    )

def draw_board(surface, font) -> None:
    """
    @brief Draws the full chessboard including margins and coordinate labels.

    Renders an 8×8 chessboard onto the given surface, including:
    - Background margin area
    - Alternating light and dark squares
    - Rank labels (1–8) along the left margin
    - File labels (a–h) along the bottom margin

    @param surface Target Pygame surface to draw onto
    @param font Pygame font object used for rendering coordinate labels

    @details
    Board Rendering:
    - Squares are drawn using a double loop over rows and columns.
    - Each square’s color is determined by (row + col) % 2.
    - Squares are offset by MARGIN_WIDTH to account for surrounding margins.

    Coordinate Labels:
    - Ranks (1–8) are displayed on the left side:
        - Top row corresponds to rank 8
        - Bottom row corresponds to rank 1
    - Files (a–h) are displayed along the bottom:
        - Leftmost column is 'a'
        - Rightmost column is 'h'

    Positioning:
    - All labels are centered relative to their corresponding squares
    - Margins provide spacing for labels outside the playable board

    @note
    - Assumes ROWS_ITER and COLS_ITER iterate over range(0, 8)
    - Relies on consistent TILE_SIZE and MARGIN_WIDTH for alignment
    - Uses write_text() for consistent text rendering and centering

    @warning
    - Does not clear or redraw dynamically changing elements (e.g., pieces)
    - Must be called every frame to maintain display consistency

    @complexity O(64) for square rendering + O(16) for labels
    """
    #Draw margins as background
    surface.fill(config.MARGIN_BG_COLOR)

    for row in config.ROWS_ITER:
        for col in config.COLS_ITER:
            #Computing color of tile based on position
            tilecol = config.LIGHT_SQUARE_COLOR if (row+col)%2 == 0 else config.DARK_SQUARE_COLOR
            #Computing dimensions and coordinate position of tile
            tiledraw = (
                col * config.TILE_SIZE + config.MARGIN_WIDTH, 
                row * config.TILE_SIZE + config.MARGIN_WIDTH, 
                config.TILE_SIZE, 
                config.TILE_SIZE
            )

            pygame.draw.rect(surface, tilecol, tiledraw)
    
    #Write rank headers on margin
    for row in config.ROWS_ITER:
        write_text(surface, font, config.MARGIN_HEADER_COLOR, str(8-row), 
                  (config.MARGIN_WIDTH/2,
                   row * config.TILE_SIZE + config.MARGIN_WIDTH+config.TILE_SIZE/2))
                    
    #Write rank headers on margin
    for col in config.COLS_ITER:
        write_text(surface, font, config.MARGIN_HEADER_COLOR, chr(ord('a')+col), 
                    (col * config.TILE_SIZE + config.MARGIN_WIDTH+config.TILE_SIZE/2, 
                    config.BOARD_SIZE + config.MARGIN_WIDTH*3/2))

def get_piece_img(piece_char:str):
    return config.IMG[config.CHAR_TO_IMG[piece_char]]

def draw_piece(surface, piece_char, pos) -> None:
    piece = get_piece_img(piece_char)
    surface.blit(piece, pos)

def remove_piece(board, row, col) -> None:
    board[row][col] = '_' #Empty Square

def place_pieces(surface, board) -> None:
    for row in config.ROWS_ITER:
        for col in config.COLS_ITER:
            if board[row][col] == '_':
                continue
            
            draw_piece(surface, board[row][col], get_tile_center_coord(row, col))
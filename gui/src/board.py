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
    """
    @brief Computes the top-left pixel coordinates for a given board tile.

    Converts board indices (row, col) into pixel-space coordinates relative
    to the rendering surface, accounting for the board margin offset.

    @param row Integer row index (0–7), where 0 is the top (rank 8)
    @param col Integer column index (0–7), where 0 is the left (file 'a')

    @return Tuple (x, y) representing the top-left pixel position of the tile

    @note
    - Coordinates are NOT centered; they represent the tile’s top-left corner
    - To center a piece, additional offset may be required
    - Assumes consistent TILE_SIZE and MARGIN_WIDTH

    @complexity O(1)
    """
    return (
        config.MARGIN_WIDTH + config.TILE_SIZE*col, #x_pos
        config.MARGIN_WIDTH + config.TILE_SIZE*row, #y_pos
    )

def get_board_indices(x_pos:int, y_pos:int):
    """
    @brief Converts pixel coordinates into board indices.

    Maps a given pixel position (typically from a mouse event) into the
    corresponding board tile indices.

    @param x_pos X-coordinate in pixels
    @param y_pos Y-coordinate in pixels

    @return Tuple (row, col) corresponding to board indices

    @note
    - Assumes input coordinates lie within the board region
    - Does not perform bounds checking (caller must validate)
    - Inverse of get_tile_center_coord (approximately)

    @warning
    - If coordinates fall within margins, result may be invalid

    @complexity O(1)
    """
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
    """
    @brief Retrieves the image surface corresponding to a piece character.

    Maps a piece identifier (e.g., 'P', 'k') to its associated Pygame image
    using configuration mappings.

    @param piece_char Character representing a chess piece:
                      - Uppercase for White
                      - Lowercase for Black

    @return Pygame Surface object representing the piece image

    @note
    - Relies on config.CHAR_TO_IMG for mapping
    - Assumes config.IMG contains preloaded image surfaces

    @warning
    - Undefined behavior if piece_char is invalid or not mapped

    @complexity O(1)
    """
    return config.IMG[config.CHAR_TO_IMG[piece_char]]

def draw_piece(surface, piece_char, pos) -> None:
    """
    @brief Draws a chess piece image onto the surface.

    Retrieves the corresponding image for the given piece character and
    blits it onto the target surface at the specified position.

    @param surface Target Pygame surface to draw onto
    @param piece_char Character representing the chess piece
    @param pos Tuple (x, y) representing the top-left position for drawing

    @note
    - Position is treated as top-left; ensure correct alignment externally
    - Uses preloaded images from config

    @complexity O(1)
    """
    piece = get_piece_img(piece_char)
    surface.blit(piece, pos)

def remove_piece(board, row, col) -> None:
    """
    @brief Removes a piece from the board at the specified location.

    Sets the given board position to an empty state ('_').

    @param board 2D list representing the board state
    @param row Row index (0–7)
    @param col Column index (0–7)

    @note
    - Mutates the board in-place
    - Does not validate indices

    @complexity O(1)
    """
    board[row][col] = '_' #Empty Square

def place_pieces(surface, board) -> None:
    """
    @brief Renders all pieces from the board onto the surface.

    Iterates over the board state and draws each non-empty piece at its
    corresponding tile position.

    @param surface Target Pygame surface to draw onto
    @param board 2D list (8x8) representing the board state

    @details
    - Skips empty squares ('_')
    - Uses get_tile_center_coord() for positioning
    - Delegates rendering to draw_piece()

    @note
    - Assumes board dimensions are 8x8
    - Assumes valid piece characters compatible with get_piece_img()

    @warning
    - Does not clear previous frame; must be called after draw_board()

    @complexity O(64) — iterates through all board squares
    """
    for row in config.ROWS_ITER:
        for col in config.COLS_ITER:
            if board[row][col] == '_':
                continue
            
            draw_piece(surface, board[row][col], get_tile_center_coord(row, col))
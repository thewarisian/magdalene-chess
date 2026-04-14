import pygame

pygame.mixer.init()

#Square tiles
LIGHT_SQUARE_COLOR = (240, 217, 181) #Light Brown
DARK_SQUARE_COLOR = (181, 136, 99) #Dark Brown
TILE_SIZE = 60
ROWS_ITER = COLS_ITER = range(0, 8)

#MARGINS
MARGIN_WIDTH = 20
MARGIN_BG_COLOR = (102, 51, 45) #Mahogany
MARGIN_HEADER_COLOR = (255, 255, 255) #White
MARGIN_FONT_STYLE = None
MARGIN_FONT_SIZE = 24

#PIECES
PIECE_SPRITE_SIZE = 60

#BOARD
BOARD_BEGIN = MARGIN_WIDTH
BOARD_END = MARGIN_WIDTH + 8*TILE_SIZE

#Display window
BOARD_SIZE = 8 * TILE_SIZE
WINDOW_SIZE = BOARD_SIZE + 2 * MARGIN_WIDTH
WINDOW_DIM = (WINDOW_SIZE, WINDOW_SIZE)
WINDOW_CENTER = (WINDOW_SIZE/2, WINDOW_SIZE/2)
WINDOW_TITLE = "Magdalene Chess Engine"

#Piece Icons
CHAR_TO_IMG = {
    'P': 'wP', 'R': 'wR', 'N': 'wN', 'B': 'wB', 'Q': 'wQ', 'K': 'wK',
    'p': 'bP', 'r': 'bR', 'n': 'bN', 'b': 'bB', 'q': 'bQ', 'k': 'bK',
}
IMG = dict()
def load_images():
    """
    @brief Loads and scales chess piece images into memory.

    Iterates through all piece identifiers defined in CHAR_TO_IMG, loads the
    corresponding image files from disk, scales them to match the tile size,
    and stores them in the global IMG dictionary for fast access during rendering.

    @details
    - Image file paths are constructed as:
        "gui/assets/icons/pieces/<piece>.png"
      where <piece> is a string like 'wP', 'bK', etc.
    - Each image is resized to (TILE_SIZE, TILE_SIZE) to fit board squares.

    @note
    - Must be called after pygame.init()
    - Should ideally be called once during initialization
    - IMG acts as a cache to avoid repeated disk I/O

    @warning
    - Will raise an error if image files are missing or paths are incorrect
    - Overwrites existing entries in IMG if called multiple times

    @complexity
    O(n) where n = number of piece types (constant ≈ 12)
    """
    for val in CHAR_TO_IMG.values():
        img = pygame.image.load(f"gui/assets/icons/pieces/{val}.png")
        img = pygame.transform.scale(img, (TILE_SIZE, TILE_SIZE))
        IMG[val] = img
load_images()

#Sounds
MOVE_SOUND = pygame.mixer.Sound('gui/assets/sounds/move-self.mp3')
CAPTURE_SOUND = pygame.mixer.Sound('gui/assets/sounds/capture.mp3')
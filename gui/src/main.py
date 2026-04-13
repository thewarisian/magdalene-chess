import pygame

import config, board

pygame.init()
layout = [
    ['r','n','b','q','k','b','n','r'],  # 8
    ['p','p','p','p','p','p','p','p'],  # 7
    ['_','_','_','_','_','_','_','_'],  # 6
    ['_','_','_','_','_','_','_','_'],  # 5
    ['_','_','_','_','_','_','_','_'],  # 4
    ['_','_','_','_','_','_','_','_'],  # 3
    ['P','P','P','P','P','P','P','P'],  # 2
    ['R','N','B','Q','K','B','N','R']   # 1
]

#Make Window
screen = pygame.display.set_mode(config.WINDOW_DIM)
pygame.display.set_caption(config.WINDOW_TITLE)

#Set fonts
font = pygame.font.SysFont(config.MARGIN_FONT_STYLE, config.MARGIN_FONT_SIZE)

running = True
#GAME LOOP
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT: running = False
    
    #Draw board
    board.draw_board(screen, font)
    board.place_pieces(screen, layout)
    
    #Display drawn items
    pygame.display.flip()

pygame.quit()
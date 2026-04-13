import pygame

import config, board

pygame.init()

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
    
    #Display drawn items
    pygame.display.flip()

pygame.quit()
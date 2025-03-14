import pygame
import math
import random

# Initialize Pygame
pygame.init()

# Screen settings
WIDTH, HEIGHT = 800, 400
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Tennis for Two")

# Colors
WHITE = (255, 255, 255)
GREEN = (0, 255, 0)

# Physics settings
gravity = 0.3
bounce_factor = 0.7

# Ball settings
ball_radius = 5
ball_x, ball_y = 100, 200
ball_speed_x, ball_speed_y = 0, 0
in_air = False

# Net position
net_x = WIDTH // 2
net_height = HEIGHT // 2

# Player settings
p1_angle = 45
p1_power = 5
p2_angle = 45
p2_power = 5
angle_step = 5
power_step = 1
max_power = 10
min_power = 10

# AI settings
ai_learning_rate = 0.1

# Font
font = pygame.font.Font(None, 24)

# Game loop
running = True
clock = pygame.time.Clock()
while running:
    screen.fill((0, 0, 0))
    pygame.draw.line(screen, GREEN, (0, HEIGHT - 50), (WIDTH, HEIGHT - 50), 5)
    pygame.draw.rect(screen, WHITE, (net_x - 5, HEIGHT - net_height, 10, net_height))
    pygame.draw.circle(screen, WHITE, (int(ball_x), int(ball_y)), ball_radius)
    
    # Display angles and power
    p1_angle_text = font.render(f"P1 Angle: {p1_angle}°", True, WHITE)
    p1_power_text = font.render(f"P1 Power: {p1_power}", True, WHITE)
    p2_angle_text = font.render(f"P2 Angle: {p2_angle}°", True, WHITE)
    p2_power_text = font.render(f"P2 Power: {p2_power}", True, WHITE)
    
    screen.blit(p1_angle_text, (10, 10))
    screen.blit(p1_power_text, (10, 30))
    screen.blit(p2_angle_text, (WIDTH - 150, 10))
    screen.blit(p2_power_text, (WIDTH - 150, 30))
    
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            # Player 1 Controls (Left Side)
            if event.key == pygame.K_w:
                p1_angle = min(90, p1_angle + angle_step)
            elif event.key == pygame.K_s:
                p1_angle = max(10, p1_angle - angle_step)
            elif event.key == pygame.K_a:
                p1_power = max(min_power, p1_power - power_step)
            elif event.key == pygame.K_d:
                p1_power = min(max_power, p1_power + power_step)
            elif event.key == pygame.K_SPACE and not in_air:
                ball_x, ball_y = 100, 200
                ball_speed_x = p1_power * math.cos(math.radians(p1_angle))
                ball_speed_y = -p1_power * math.sin(math.radians(p1_angle))
                in_air = True
    
    # AI Player 2 (Right Side)
    if not in_air and random.random() < 0.02:  # AI decides to hit
        p2_angle += random.choice([-angle_step, angle_step]) * ai_learning_rate
        p2_power += random.choice([-power_step, power_step]) * ai_learning_rate
        p2_angle = max(10, min(90, p2_angle))
        p2_power = max(min_power, min(max_power, p2_power))
        
        ball_x, ball_y = WIDTH - 100, 200
        ball_speed_x = -p2_power * math.cos(math.radians(p2_angle))
        ball_speed_y = -p2_power * math.sin(math.radians(p2_angle))
        in_air = True
    
    if in_air:
        ball_x += ball_speed_x
        ball_y += ball_speed_y
        ball_speed_y += gravity
        
        if ball_y >= HEIGHT - 50 - ball_radius:
            ball_y = HEIGHT - 50 - ball_radius
            ball_speed_y = -ball_speed_y * bounce_factor
            if abs(ball_speed_y) < 1:
                in_air = False
    
    pygame.display.flip()
    clock.tick(60)

pygame.quit()

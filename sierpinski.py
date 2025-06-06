import matplotlib.pyplot as plt
import numpy as np

def draw_triangle(ax, points, color='black'):
    triangle = plt.Polygon(points, edgecolor=color, fill=None)
    ax.add_patch(triangle)

def sierpinski(ax, points, depth):
    if depth == 0:
        draw_triangle(ax, points)
    else:
        # Midpoints of each side
        mid = lambda p1, p2: [(p1[0]+p2[0])/2, (p1[1]+p2[1])/2]
        p1, p2, p3 = points
        m1 = mid(p1, p2)
        m2 = mid(p2, p3)
        m3 = mid(p3, p1)
        
        sierpinski(ax, [p1, m1, m3], depth-1)
        sierpinski(ax, [m1, p2, m2], depth-1)
        sierpinski(ax, [m3, m2, p3], depth-1)

# Main setup
fig, ax = plt.subplots()
ax.set_aspect('equal')
ax.axis('off')

# Define base triangle
points = [[0, 0], [1, 0], [0.5, np.sqrt(3)/2]]
sierpinski(ax, points, depth=6)

plt.show()

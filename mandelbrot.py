import numpy as np
import matplotlib.pyplot as plt

def mandelbrot(c, max_iter):
    z = 0
    for n in range(max_iter):
        if abs(z) > 2:
            return n
        z = z*z + c
    return max_iter

# Image size and bounds
width, height = 800, 600
xmin, xmax = -2, 1
ymin, ymax = -1.5, 1.5
max_iter = 100

# Create image
image = np.zeros((height, width))

for x in range(width):
    for y in range(height):
        real = xmin + (x / width) * (xmax - xmin)
        imag = ymin + (y / height) * (ymax - ymin)
        c = complex(real, imag)
        color = mandelbrot(c, max_iter)
        image[y, x] = color

plt.imshow(image, extent=(xmin, xmax, ymin, ymax), cmap='hot')
plt.colorbar()
plt.title("Mandelbrot Set")
plt.show()

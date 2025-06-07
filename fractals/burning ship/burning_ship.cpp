#include <iostream>
#include <fstream>
#include <complex>
#include <sstream>
#include <cmath>
#include <omp.h>
#include <vector>

struct Color {
    int r, g, b;
    Color(int r = 0, int g = 0, int b = 0) : r(r), g(g), b(b) {}
};

Color getColor(int iter, int max_iter, double smooth_iter) {
    if (iter == max_iter) return Color(0, 0, 0); 

    double t = (iter + 1 - smooth_iter) / max_iter;
    t = std::pow(t, 0.5);
    if (t < 0.16) {
        // Deep red to orange
        double local_t = t / 0.16;
        return Color(
            (int)(50 + 155 * local_t),
            (int)(10 + 40 * local_t),
            (int)(5 + 10 * local_t)
        );
    } else if (t < 0.42) {
        // Orange to bright yellow
        double local_t = (t - 0.16) / 0.26;
        return Color(
            (int)(205 + 50 * local_t),
            (int)(50 + 155 * local_t),
            (int)(15 + 25 * local_t)
        );
    } else if (t < 0.64) {
        // Yellow to white hot
        double local_t = (t - 0.42) / 0.22;
        return Color(
            255,
            (int)(205 + 50 * local_t),
            (int)(40 + 115 * local_t)
        );
    } else if (t < 0.86) {
        // White to pale blue
        double local_t = (t - 0.64) / 0.22;
        return Color(
            (int)(255 - 55 * local_t),
            (int)(255 - 55 * local_t),
            (int)(155 + 100 * local_t)
        );
    } else {
        // Pale blue to deep blue
        double local_t = (t - 0.86) / 0.14;
        return Color(
            (int)(200 - 150 * local_t),
            (int)(200 - 150 * local_t),
            255
        );
    }
}

int main() {
    const int width = 1200; 
    const int height = 900;
    const int max_iter = 2000;  
  
    const double x_min = -2.0;
    const double x_max = 1.0;
    const double y_min = -1.5;
    const double y_max = 1.0;
    
    std::ofstream img("burning_ship_enhanced.ppm");
    img << "P3\n" << width << " " << height << "\n255\n";

    std::vector<std::vector<Color>> buffer(height, std::vector<Color>(width));
    
    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Map pixel to complex plane with better scaling
            double real = x_min + (x * (x_max - x_min)) / width;
            double imag = y_min + (y * (y_max - y_min)) / height;
            
            double zx = 0.0, zy = 0.0;
            int iter = 0;

            while (zx * zx + zy * zy <= 256.0 && iter < max_iter) {  
                double zx_new = zx * zx - zy * zy + real;
                double zy_new = std::abs(2.0 * zx * zy) + imag;  
                zx = std::abs(zx_new);  // abs() on real part too
                zy = zy_new;
                iter++;
            }

            double smooth_iter = 0.0;
            if (iter < max_iter) {
                double log_zn = std::log(zx * zx + zy * zy) / 2.0;
                double nu = std::log(log_zn / std::log(2.0)) / std::log(2.0);
                smooth_iter = nu;
            }
            
            buffer[y][x] = getColor(iter, max_iter, smooth_iter);
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            img << buffer[y][x].r << " " << buffer[y][x].g << " " << buffer[y][x].b << "\n";
        }
    }
    
    img.close();
    std::cout << "Resolution: " << width << "x" << height << ", Max iterations: " << max_iter << std::endl;
    
    return 0;
}

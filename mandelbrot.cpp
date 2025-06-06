#include <iostream>
#include <fstream>
#include <complex>
#include <omp.h>

int main() {
    const int width = 800;
    const int height = 600;
    const int max_iter = 100;

    std::ofstream img("mandelbrot.ppm");
    img << "P3\n" << width << " " << height << "\n255\n";

    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; ++y) {
        std::stringstream row;
        for (int x = 0; x < width; ++x) {
            std::complex<double> c(
                (x - width / 2.0) * 4.0 / width,
                (y - height / 2.0) * 4.0 / height
            );
            std::complex<double> z = 0;
            int iter = 0;
            while (abs(z) <= 2.0 && iter < max_iter) {
                z = z * z + c;
                iter++;
            }

            int color = 255 * iter / max_iter;
            row << color << " " << 0 << " " << 0 << "\n";
        }

        #pragma omp critical
        img << row.str();
    }

    img.close();
    return 0;
}

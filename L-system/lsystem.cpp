#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include <cmath>
#include <fstream>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Vec3 {
    float x, y, z;
    
    Vec3(float x_=0, float y_=0, float z_=0) : x(x_), y(y_), z(z_) {}
    
    Vec3 operator+(const Vec3& b) const { return Vec3(x+b.x, y+b.y, z+b.z); }
    Vec3 operator-(const Vec3& b) const { return Vec3(x-b.x, y-b.y, z-b.z); }
    Vec3 operator*(float s) const { return Vec3(x*s, y*s, z*s); }
    
    float dot(const Vec3& b) const { return x*b.x + y*b.y + z*b.z; }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    
    Vec3 normalize() const {
        float len = length();
        return len > 0 ? (*this) * (1.0f / len) : Vec3();
    }
    
    Vec3 cross(const Vec3& b) const {
        return Vec3(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x);
    }
};

struct Matrix3 {
    float m[3][3];
    
    Matrix3() {
        // Identity matrix
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }
    
    Vec3 operator*(const Vec3& v) const {
        return Vec3(
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
        );
    }
    
    Matrix3 operator*(const Matrix3& other) const {
        Matrix3 result;
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                result.m[i][j] = 0;
                for(int k = 0; k < 3; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
    
    static Matrix3 rotationX(float angle) {
        Matrix3 mat;
        float c = std::cos(angle);
        float s = std::sin(angle);
        mat.m[1][1] = c; mat.m[1][2] = -s;
        mat.m[2][1] = s; mat.m[2][2] = c;
        return mat;
    }
    
    static Matrix3 rotationY(float angle) {
        Matrix3 mat;
        float c = std::cos(angle);
        float s = std::sin(angle);
        mat.m[0][0] = c; mat.m[0][2] = s;
        mat.m[2][0] = -s; mat.m[2][2] = c;
        return mat;
    }
    
    static Matrix3 rotationZ(float angle) {
        Matrix3 mat;
        float c = std::cos(angle);
        float s = std::sin(angle);
        mat.m[0][0] = c; mat.m[0][1] = -s;
        mat.m[1][0] = s; mat.m[1][1] = c;
        return mat;
    }
};

struct TurtleState {
    Vec3 position;
    Vec3 heading;     // Forward direction
    Vec3 left;        // Left direction
    Vec3 up;          // Up direction
    float lineWidth;
    Vec3 color;
    
    TurtleState() : position(0,0,0), heading(0,1,0), left(-1,0,0), up(0,0,1), 
                   lineWidth(1.0f), color(0.2f, 0.8f, 0.2f) {}
};

struct Line {
    Vec3 start, end;
    float width;
    Vec3 color;
};

class LSystem {
private:
    std::string axiom;
    std::unordered_map<char, std::string> rules;
    std::string current;
    
public:
    LSystem(const std::string& ax) : axiom(ax), current(ax) {}
    
    void addRule(char symbol, const std::string& replacement) {
        rules[symbol] = replacement;
    }
    
    void iterate(int generations) {
        current = axiom;
        for(int gen = 0; gen < generations; gen++) {
            std::string next;
            for(char c : current) {
                if(rules.find(c) != rules.end()) {
                    next += rules[c];
                } else {
                    next += c;
                }
            }
            current = next;
            std::cout << "Generation " << gen + 1 << ": Length = " << current.length() << std::endl;
        }
    }
    
    const std::string& getString() const { return current; }
};

Vec3 hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - std::abs(std::fmod(h / 60.0f, 2) - 1));
    float m = v - c;
    
    Vec3 rgb;
    if (h >= 0 && h < 60) {
        rgb = Vec3(c, x, 0);
    } else if (h >= 60 && h < 120) {
        rgb = Vec3(x, c, 0);
    } else if (h >= 120 && h < 180) {
        rgb = Vec3(0, c, x);
    } else if (h >= 180 && h < 240) {
        rgb = Vec3(0, x, c);
    } else if (h >= 240 && h < 300) {
        rgb = Vec3(x, 0, c);
    } else {
        rgb = Vec3(c, 0, x);
    }
    
    return Vec3(rgb.x + m, rgb.y + m, rgb.z + m);
}

class TurtleRenderer {
private:
    std::vector<Line> lines;
    std::stack<TurtleState> stateStack;
    TurtleState turtle;
    float angle;
    float stepSize;
    float widthFactor;
    
public:
    TurtleRenderer(float ang = 25.0f, float step = 1.0f) 
        : angle(ang * M_PI / 180.0f), stepSize(step), widthFactor(0.9f) {}
    
    void setAngle(float ang) { angle = ang * M_PI / 180.0f; }
    void setStep(float step) { stepSize = step; }
    void setWidthFactor(float factor) { widthFactor = factor; }
    
    void interpret(const std::string& lstring) {
        lines.clear();
        while(!stateStack.empty()) stateStack.pop();
        
        turtle = TurtleState();
        
        for(char c : lstring) {
            switch(c) {
                case 'F': case 'A': case 'B':
                    // Draw forward
                    drawForward();
                    break;
                    
                case 'f':
                    // Move forward without drawing
                    moveForward();
                    break;
                    
                case '+':
                    // Turn left around up axis
                    turnLeft();
                    break;
                    
                case '-':
                    // Turn right around up axis
                    turnRight();
                    break;
                    
                case '&':
                    // Pitch down around left axis
                    pitchDown();
                    break;
                    
                case '^':
                    // Pitch up around left axis
                    pitchUp();
                    break;
                    
                case '\\':
                    // Roll left around heading axis
                    rollLeft();
                    break;
                    
                case '/':
                    // Roll right around heading axis
                    rollRight();
                    break;
                    
                case '|':
                    // Turn around 180 degrees
                    turnAround();
                    break;
                    
                case '[':
                    // Push state
                    stateStack.push(turtle);
                    turtle.lineWidth *= widthFactor;
                    break;
                    
                case ']':
                    // Pop state
                    if(!stateStack.empty()) {
                        turtle = stateStack.top();
                        stateStack.pop();
                    }
                    break;
                    
                case '!':
                    // Decrease line width
                    turtle.lineWidth *= 0.7f;
                    break;
                    
                case '<':
                    // Multiply line width
                    turtle.lineWidth *= 1.4f;
                    break;
                    
                case '>':
                    // Divide line width
                    turtle.lineWidth *= 0.7f;
                    break;
                    
                default:
                    // Ignore unknown symbols
                    break;
            }
        }
        
        std::cout << "Generated " << lines.size() << " line segments" << std::endl;
    }

    
    
private:
    void drawForward() {
        Vec3 newPos = turtle.position + turtle.heading * stepSize;
        lines.push_back({turtle.position, newPos, turtle.lineWidth, turtle.color});
        turtle.position = newPos;
    }
    
    void moveForward() {
        turtle.position = turtle.position + turtle.heading * stepSize;
    }
    
    void turnLeft() {
        Matrix3 rot = Matrix3::rotationZ(angle);
        turtle.heading = rot * turtle.heading;
        turtle.left = rot * turtle.left;
    }
    
    void turnRight() {
        Matrix3 rot = Matrix3::rotationZ(-angle);
        turtle.heading = rot * turtle.heading;
        turtle.left = rot * turtle.left;
    }
    
    void pitchDown() {
        Matrix3 rot = Matrix3::rotationX(angle);
        turtle.heading = rot * turtle.heading;
        turtle.up = rot * turtle.up;
    }
    
    void pitchUp() {
        Matrix3 rot = Matrix3::rotationX(-angle);
        turtle.heading = rot * turtle.heading;
        turtle.up = rot * turtle.up;
    }
    
    void rollLeft() {
        Matrix3 rot = Matrix3::rotationY(angle);
        turtle.left = rot * turtle.left;
        turtle.up = rot * turtle.up;
    }
    
    void rollRight() {
        Matrix3 rot = Matrix3::rotationY(-angle);
        turtle.left = rot * turtle.left;
        turtle.up = rot * turtle.up;
    }
    
    void turnAround() {
        turtle.heading = turtle.heading * -1.0f;
        turtle.left = turtle.left * -1.0f;
    }
    
public:

    void renderToSVGEnhanced(const std::string& filename, float scale = 10.0f, 
                            const std::string& style = "gradient") {
        if(lines.empty()) return;
        
        // Find bounding box
        Vec3 minBounds = lines[0].start;
        Vec3 maxBounds = lines[0].start;
        
        for(const Line& line : lines) {
            minBounds.x = std::min(minBounds.x, std::min(line.start.x, line.end.x));
            minBounds.y = std::min(minBounds.y, std::min(line.start.y, line.end.y));
            maxBounds.x = std::max(maxBounds.x, std::max(line.start.x, line.end.x));
            maxBounds.y = std::max(maxBounds.y, std::max(line.start.y, line.end.y));
        }
        
        float width = (maxBounds.x - minBounds.x) * scale + 200;
        float height = (maxBounds.y - minBounds.y) * scale + 200;
        
        std::ofstream file(filename);
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
        
        // Enhanced background with gradient
        file << "<defs>\n";
        
        if(style == "gradient" || style == "rainbow") {
            // Background gradient
            file << "<radialGradient id=\"bg\" cx=\"50%\" cy=\"50%\" r=\"50%\">\n";
            file << "<stop offset=\"0%\" style=\"stop-color:#001122;stop-opacity:1\" />\n";
            file << "<stop offset=\"100%\" style=\"stop-color:#000000;stop-opacity:1\" />\n";
            file << "</radialGradient>\n";
            
            // Line gradients for depth effect
            for(int i = 0; i < std::min((int)lines.size(), 10); i++) {
                file << "<linearGradient id=\"lineGrad" << i << "\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">\n";
                
                if(style == "rainbow") {
                    float hue1 = std::fmod(i * 137.5f, 360.0f);
                    float hue2 = std::fmod(hue1 + 60.0f, 360.0f);
                    Vec3 color1 = hsvToRgb(hue1, 0.8f, 0.9f);
                    Vec3 color2 = hsvToRgb(hue2, 0.6f, 0.7f);
                    
                    file << "<stop offset=\"0%\" style=\"stop-color:rgb(" 
                         << (int)(color1.x*255) << "," << (int)(color1.y*255) << "," << (int)(color1.z*255) << ");stop-opacity:0.9\" />\n";
                    file << "<stop offset=\"100%\" style=\"stop-color:rgb(" 
                         << (int)(color2.x*255) << "," << (int)(color2.y*255) << "," << (int)(color2.z*255) << ");stop-opacity:0.6\" />\n";
                } else {
                    // Green to yellow gradient for organic look
                    file << "<stop offset=\"0%\" style=\"stop-color:#90EE90;stop-opacity:0.9\" />\n";
                    file << "<stop offset=\"100%\" style=\"stop-color:#32CD32;stop-opacity:0.6\" />\n";
                }
                file << "</linearGradient>\n";
            }
            
            // Glow filter
            file << "<filter id=\"glow\">\n";
            file << "<feGaussianBlur stdDeviation=\"2\" result=\"coloredBlur\"/>\n";
            file << "<feMerge>\n";
            file << "<feMergeNode in=\"coloredBlur\"/>\n";
            file << "<feMergeNode in=\"SourceGraphic\"/>\n";
            file << "</feMerge>\n";
            file << "</filter>\n";
        }
        
        file << "</defs>\n";
        
        // Background
        if(style == "gradient" || style == "rainbow") {
            file << "<rect width=\"100%\" height=\"100%\" fill=\"url(#bg)\"/>\n";
        } else if(style == "light") {
            file << "<rect width=\"100%\" height=\"100%\" fill=\"#f8f8f8\"/>\n";
        } else {
            file << "<rect width=\"100%\" height=\"100%\" fill=\"black\"/>\n";
        }
        
        // Render lines with enhanced styling
        for(int i = 0; i < lines.size(); i++) {
            const Line& line = lines[i];
            float x1 = (line.start.x - minBounds.x) * scale + 100;
            float y1 = height - ((line.start.y - minBounds.y) * scale + 100);
            float x2 = (line.end.x - minBounds.x) * scale + 100;
            float y2 = height - ((line.end.y - minBounds.y) * scale + 100);
            
            std::string strokeColor;
            std::string filter = "";
            float strokeWidth = std::max(0.5f, line.width * 0.8f);
            
            if(style == "rainbow") {
                float hue = std::fmod(i * 137.5f, 360.0f);
                Vec3 color = hsvToRgb(hue, 0.7f, 0.9f);
                strokeColor = "rgb(" + std::to_string((int)(color.x*255)) + "," + 
                             std::to_string((int)(color.y*255)) + "," + 
                             std::to_string((int)(color.z*255)) + ")";
                filter = " filter=\"url(#glow)\"";
            } else if(style == "gradient") {
                strokeColor = "url(#lineGrad" + std::to_string(i % 10) + ")";
                filter = " filter=\"url(#glow)\"";
            } else if(style == "light") {
                int r = (int)(line.color.x * 180 + 75);
                int g = (int)(line.color.y * 180 + 75);
                int b = (int)(line.color.z * 180 + 75);
                strokeColor = "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
            } else if(style == "depth") {
                // Color based on distance from center for depth effect
                float centerX = width / 2;
                float centerY = height / 2;
                float dist = std::sqrt((x1-centerX)*(x1-centerX) + (y1-centerY)*(y1-centerY));
                float maxDist = std::sqrt(centerX*centerX + centerY*centerY);
                float intensity = 1.0f - (dist / maxDist) * 0.7f;
                
                int r = (int)(line.color.x * 255 * intensity);
                int g = (int)(line.color.y * 255 * intensity);
                int b = (int)(line.color.z * 255 * intensity);
                strokeColor = "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
                strokeWidth *= intensity;
            } else {
                // Default style
                int r = (int)(line.color.x * 255);
                int g = (int)(line.color.y * 255);
                int b = (int)(line.color.z * 255);
                strokeColor = "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
            }
            
            file << "<line x1=\"" << x1 << "\" y1=\"" << y1 
                 << "\" x2=\"" << x2 << "\" y2=\"" << y2 
                 << "\" stroke=\"" << strokeColor
                 << "\" stroke-width=\"" << strokeWidth 
                 << "\" stroke-linecap=\"round\""
                 << filter << "/>\n";
        }
        
        file << "</svg>\n";
        file.close();
        std::cout << "Enhanced SVG file saved: " << filename << std::endl;
    }
    
    void renderToSVG(const std::string& filename, float scale = 10.0f) {
        if(lines.empty()) return;
        
        // Find bounding box
        Vec3 minBounds = lines[0].start;
        Vec3 maxBounds = lines[0].start;
        
        for(const Line& line : lines) {
            minBounds.x = std::min(minBounds.x, std::min(line.start.x, line.end.x));
            minBounds.y = std::min(minBounds.y, std::min(line.start.y, line.end.y));
            maxBounds.x = std::max(maxBounds.x, std::max(line.start.x, line.end.x));
            maxBounds.y = std::max(maxBounds.y, std::max(line.start.y, line.end.y));
        }
        
        float width = (maxBounds.x - minBounds.x) * scale + 100;
        float height = (maxBounds.y - minBounds.y) * scale + 100;
        
        std::ofstream file(filename);
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
        file << "<rect width=\"100%\" height=\"100%\" fill=\"black\"/>\n";
        
        for(const Line& line : lines) {
            float x1 = (line.start.x - minBounds.x) * scale + 50;
            float y1 = height - ((line.start.y - minBounds.y) * scale + 50);
            float x2 = (line.end.x - minBounds.x) * scale + 50;
            float y2 = height - ((line.end.y - minBounds.y) * scale + 50);
            
            int r = (int)(line.color.x * 255);
            int g = (int)(line.color.y * 255);
            int b = (int)(line.color.z * 255);
            
            file << "<line x1=\"" << x1 << "\" y1=\"" << y1 
                 << "\" x2=\"" << x2 << "\" y2=\"" << y2 
                 << "\" stroke=\"rgb(" << r << "," << g << "," << b 
                 << ")\" stroke-width=\"" << (line.width * 0.5f) << "\"/>\n";
        }
        
        file << "</svg>\n";
        file.close();
        std::cout << "SVG file saved: " << filename << std::endl;
    }
};

// Predefined L-Systems
namespace LSystems {
    LSystem dragonCurve() {
        LSystem ls("F");
        ls.addRule('F', "F+G");
        ls.addRule('G', "F-G");
        return ls;
    }
    
    LSystem sierpinskiTriangle() {
        LSystem ls("F-G-G");
        ls.addRule('F', "F-G+F+G-F");
        ls.addRule('G', "GG");
        return ls;
    }
    
    LSystem kochCurve() {
        LSystem ls("F");
        ls.addRule('F', "F+F-F-F+F");
        return ls;
    }
    
    LSystem plantA() {
        LSystem ls("F");
        ls.addRule('F', "F[+F]F[-F]F");
        return ls;
    }
    
    LSystem plantB() {
        LSystem ls("F");
        ls.addRule('F', "FF-[-F+F+F]+[+F-F-F]");
        return ls;
    }
    
    LSystem tree3D() {
        LSystem ls("A");
        ls.addRule('A', "!![&FL!A]/////'/[&FL!A]///////[&FL!A]");
        ls.addRule('F', "S //// F");
        ls.addRule('S', "F L");
        ls.addRule('L', "['''^^{-f+f+f-|-f+f+f}]");
        return ls;
    }
    
    LSystem hilbertCurve3D() {
        LSystem ls("A");
        ls.addRule('A', "B-F+CFC+F-D&F^D-F+&&CFC+F+B//");
        ls.addRule('B', "A&F^CFB^F^D^^-F-D^|F^B|FC^F^A//");
        ls.addRule('C', "|D^|F^B-F+C^F^A&&FA&F^C+F+B^F^D//");
        ls.addRule('D', "|CFB-F+B|FA&F^A&&FB-F+B|FC//");
        return ls;
    }
}



int main() {
    std::cout << "Enhanced L-System Renderer\n";
    std::cout << "=========================\n\n";
    
    // Example 1: Dragon Curve - Multiple styles
    std::cout << "1. Dragon Curve (Multiple Styles):\n";
    LSystem dragon = LSystems::dragonCurve();
    dragon.iterate(12);
    
    TurtleRenderer renderer1(90.0f, 1.0f);
    renderer1.interpret(dragon.getString());
    renderer1.renderToSVG("dragon_original.svg", 3.0f);              // Original
    renderer1.renderToSVGEnhanced("dragon_rainbow.svg", 3.0f, "rainbow");   // Rainbow
    renderer1.renderToSVGEnhanced("dragon_gradient.svg", 3.0f, "gradient"); // Gradient
    
    // Example 2: Plant A - Enhanced versions
    std::cout << "\n2. Plant A (Enhanced):\n";
    LSystem plant = LSystems::plantA();
    plant.iterate(6);
    
    TurtleRenderer renderer2(22.0f, 1.0f);
    renderer2.interpret(plant.getString());
    renderer2.renderToSVGEnhanced("plant_gradient.svg", 12.0f, "gradient");
    renderer2.renderToSVGEnhanced("plant_light.svg", 12.0f, "light");
    //    renderer2.renderToPLY("plant_a.ply");
    
    // Example 3: 3D Tree
    std::cout << "\n3. 3D Tree:\n";
    LSystem tree = LSystems::tree3D();
    tree.iterate(3);
    
    TurtleRenderer renderer3(22.5f, 1.0f);
    renderer3.setWidthFactor(0.8f);
    renderer3.interpret(tree.getString());
    renderer3.renderToSVGEnhanced("tree_depth.svg", 8.0f, "depth");
    // renderer3.renderToOBJ("tree_3d.obj");
    // renderer3.renderToPLY("tree_3d.ply");
    
    // Example 4: Koch Curve - Rainbow style
    std::cout << "\n4. Koch Curve (Rainbow):\n";
    LSystem koch = LSystems::kochCurve();
    koch.iterate(5);
    
    TurtleRenderer renderer4(90.0f, 1.0f);
    renderer4.interpret(koch.getString());
    renderer4.renderToSVGEnhanced("koch_rainbow.svg", 8.0f, "rainbow");
    renderer4.renderToSVGEnhanced("koch_depth.svg", 8.0f, "depth");
    
    // Example 5: Custom Branching Pattern - Multiple styles
    std::cout << "\n5. Custom Branching Pattern:\n";
    LSystem custom("F");
    custom.addRule('F', "F[+F][-F]FF");
    custom.iterate(5);
    
    TurtleRenderer renderer5(25.0f, 1.0f);
    renderer5.interpret(custom.getString());
    renderer5.renderToSVGEnhanced("custom_gradient.svg", 15.0f, "gradient");
    renderer5.renderToSVGEnhanced("custom_light.svg", 15.0f, "light");
    
    // Example 6: Sierpinski Triangle - Special case
    std::cout << "\n6. Sierpinski Triangle (Rainbow):\n";
    LSystem sierpinski = LSystems::sierpinskiTriangle();
    sierpinski.iterate(6);
    
    TurtleRenderer renderer6(120.0f, 1.0f);
    renderer6.interpret(sierpinski.getString());
    renderer6.renderToSVGEnhanced("sierpinski_rainbow.svg", 10.0f, "rainbow");
    
    std::cout << "\nAll enhanced L-systems rendered successfully!\n";
    std::cout << "Files generated:\n";
    std::cout << "- Dragon: dragon_original.svg, dragon_rainbow.svg, dragon_gradient.svg\n";
    std::cout << "- Plant: plant_gradient.svg, plant_light.svg, plant_a.ply\n";
    std::cout << "- Tree: tree_depth.svg, tree_3d.obj, tree_3d.ply\n";
    std::cout << "- Koch: koch_rainbow.svg, koch_depth.svg\n";
    std::cout << "- Custom: custom_gradient.svg, custom_light.svg\n";
    std::cout << "- Sierpinski: sierpinski_rainbow.svg\n";
    
    return 0;
}
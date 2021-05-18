#pragma once

#include <list>

class Plot {
private:
    class Tick {
        public:
            char* text;
            float value;
    };

    int width;
    int height;
    int axWidth;
    int axHeight;
    int axXCoord;
    int axYCoord;
    float xlim[2];
    float ylim[2];
    float ylimAlt[2];
    float lineWidth;
    u32 bgColor;
    std::list<Tick> xTicks;
    std::list<Tick> yTicks;
    std::list<Tick> yTicksAlt;
    Font* font;
public:
    Plot();
    Plot(int _width, int _height);
    ~Plot();

    void setXlim(float x1, float x2);
    void setYlim(float y1, float y2);
    void setYlimAlt(float y1, float y2);
    void setSize(int _width, int _height);
    void setAxSize(int _width, int _height);
    void setAxPos(int _x, int _y);
    void setLineWidth(float _lineWidth);
    void setBackgroundColor(u32 _color);
    void addXTick(const char* _text, float _value);
    void addYTick(const char* _text, float _value);
    void addYTickAlt(const char* _text, float _value);
    void setFont(Font* _font);

    void plot(float* x, float* y, u32 nEl, bool computeLimits, u32 color);
    void plotAlt(float* x, float* y, u32 nEl, bool computeLimits, u32 color);

};

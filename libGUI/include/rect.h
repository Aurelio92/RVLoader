#pragma once

#include <algorithm>

class Rect {
    public:
        float x, y, width, height;

        Rect() {
            x = 0;
            y = 0;
            width = 0;
            height = 0;
        }

        Rect(float _x, float _y, float _w, float _h) {
            x = _x;
            y = _y;
            width = _w;
            height = _h;
        }

        Rect(const Rect& r) {
            x = r.x;
            y = r.y;
            width = r.width;
            height = r.height;
        }

        bool isInside(float _x, float _y) {
            return (_x >= x && _x < x + width && _y >= y && _y < y + height);
        }

        bool intersection(Rect& out, const Rect rect) {
            bool intersect = x < rect.x + rect.width && rect.x < x + width && y < rect.y + rect.height && rect.y < y + height;

            if (intersect) {
                float left = std::max(x, rect.x);
                float top = std::max(y, rect.y);
                float right = std::min(x + width, rect.x + rect.width);
                float bottom = std::min(y + height, rect.y + rect.height);

                out = Rect(left, top, right - left, bottom - top);
            } else {
                out = Rect(0, 0, 0, 0);
            }

            return intersect;
        }
};

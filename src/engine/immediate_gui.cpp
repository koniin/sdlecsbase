#include "immediate_gui.h"
#include <vector>
#include "renderer.h"

namespace IGUI {
    struct NumberEditI {
        std::string label;
        int *val;
        int min;
        int max;
        int step;
        Point size;
    };

    std::vector<NumberEditI> number_edits;
    
    void click_check(Rectangle &rect, std::function<void(void)> on_click) {
        Point p;
        Input::mouse_current(p);
        if(rect.contains(p)) {
            if(Input::mouse_left_down) {
                if(on_click != nullptr) {
                    on_click();
                }
            }
        }
    }

    void frame() {
        number_edits.clear();
    }

    void number_edit_i(std::string label, int *i, int min, int max, int step) {
        NumberEditI edit = { label, i, min, max, step };
        std::string max_text = label + " 999999"; 
        TextCache::size(max_text.c_str(), &edit.size.x, &edit.size.y);
        number_edits.push_back(edit);
    }

    void render() {
        for(auto nei : number_edits) {
            int x = 100;
            int y = 100;
            draw_g_rectangle_filled_RGBA(x - (nei.size.x / 2) - 10, y - (nei.size.y / 2) - 10, nei.size.x + 20, nei.size.y + 20, 0, 200, 230, 255);
            
            std::string txt = nei.label + " " + std::to_string(*nei.val);
            draw_text_centered_str(100, 100, {255, 255, 255, 255 }, txt);
            
        }
    }
}

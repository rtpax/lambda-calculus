#include "component.h"
#include "compile.h"
#include "token.h"
#include <deque>

namespace lambda {

class display {
private:
    static void init();
    static bool shown; //only one can be shown at a time
    std::vector<step_string_info> choices;
    int choices_index;
    std::string choices_string;
    std::deque<component> history;
    std::deque<component>::iterator focus;
    struct binding {
        int key;
        std::function<int(void*)> function;
    };
    std::vector<binding> bindings;
public:
    display();
    ~display();

    int show();
    int hide();
    int update();//call this periodically
    int draw();
    int set_focus(component);
    component get_focus();
    int bind_key(int key, std::function<int(void*)>);
}

}







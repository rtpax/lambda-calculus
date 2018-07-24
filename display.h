#include "component.h"
#include "compile.h"
#include "token.h"

namespace lambda {

class display {
private:
    static void init();
    static bool shown; //only one can be shown at a time
    component focus;
    std::vector<step_string_info> choices;
    int choices_index;
    std::string choices_string;
    std::vector<std::pair<int,std::function<int(void*)>>> bindings;
public:
    display();
    ~display();

    int show();
    int hide();
    int update();
    int draw();
    int set_focus(component);
    int bind_key(int key, std::function<int(void*)>);
}

}

























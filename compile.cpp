#include "compile.h"

struct BINDSTRING_OS

expression_pair do_normal_form(expression in, )

expression_pair normal_form(expression in) {
    switch(in.base.id_f) {
    case identifier_flag::none:
        return expression_pair{{identifier_flag::none}};
    case identifier_flag::free:
        return expression_pair{
            {in.base.id_f,in.base.name,in.base.value.null()?nullable<lambda>():normal_form(in.base.value)},
            normal_form(in.sub)
        };
    case identifier_flag::bound:
        
    } 
}

lambda_pair normal_form(lambda) {

}

/*

A := Lxy.x(yx)
B := Lx.Axx
B = Lx.x(xx)



*/
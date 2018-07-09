#include "../component.h"
#include <iostream>

using namespace lambda;


int main() {
    component a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;

    d.lambda(e.id("w"),
        a.lambda(b.id("s"),
            c.lambda(d.id("z"),
                e.expr(f.id("s"),
                    g.expr(h.expr(i.id("w"),j.id("s")),k.id("z"))))));
    while(d.simplify_step());
    global.add_value("S",d);
    

    a.lambda(b.id("s"),
        c.lambda(a.id("z"),
            b.id("z")));
    while(a.simplify_step());
    global.add_value("0", a);
    
    a.expr(b.id("S",&prepkg::global),
        c.id("0",&prepkg::global));
    while(a.simplify_step());
    global.add_value("1", a);

    a.expr(b.id("S",&prepkg::global),
        c.id("1",&prepkg::global));
    while(a.simplify_step());
    global.add_value("2", a);

    a.expr(b.id("S",&prepkg::global),
        c.id("2",&prepkg::global));
    while(a.simplify_step());
    global.add_value("3", a);

    a.expr(b.id("S",&prepkg::global),
        c.id("3",&prepkg::global));
    while(a.simplify_step());
    global.add_value("4", a);


    a.lambda(
        b.id("x"),
        c.lambda(
            d.id("y"),
            e.expr(
                f.expr(
                    g.id("x"),
                    h.id("y")),
                i.expr(
                    j.id("y"),
                    k.id("x")))));

    z.expr(
        a,
        b.lambda(
            c.id("a"),
            d.lambda(
                e.id("b"),
                f.id("b"))));

    bool stepped = 1;
    int max_iter = 100;
    while(stepped && max_iter > 0) {
        std::cout << z.to_string() << "\n";
        stepped = z.evaluate_step();
        if(!stepped)
            stepped = z.simplify_step();
        --max_iter;
    }

    z.expr(a.expr(b.id("4",&prepkg::global),c.id("S",&prepkg::global)),d.id("4",&prepkg::global));    

    std::cout << "\n";
    stepped = 1;
    max_iter = 100;
    while(stepped && max_iter > 0) {
        std::cout << z.to_string() << "\n";
        stepped = z.evaluate_step();
        if(!stepped)
            stepped = z.simplify_step();
        --max_iter;
    }

    z.id("4",&prepkg::global);

    std::cout << "\n";
    stepped = 1;
    max_iter = 100;
    while(stepped && max_iter > 0) {
        std::cout << z.to_string() << "\n";
        stepped = z.evaluate_step();
        if(!stepped)
            stepped = z.simplify_step();
        --max_iter;
    }

    z.lambda(
        component().id("y"),
        component().lambda(
            component().id("x"),
            component().expr(
                component().id("y"),
                component().expr(
                    component().lambda(
                        component().id("y"),
                        component().lambda(
                            component().id("x"),
                            component().expr(
                                component().id("x"),
                                component().id("y")
                            )
                        )
                    ),
                    component().id("x")
                )
            )
        )
    );

    std::cout << "\n";
    stepped = 1;
    max_iter = 100;
    while(stepped && max_iter > 0) {
        std::cout << z.to_string() << "\n";
        stepped = z.evaluate_step();
        if(!stepped)
            stepped = z.simplify_step();
        --max_iter;
    }

}
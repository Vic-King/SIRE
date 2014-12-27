#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "gnuplot_i.hpp"

using namespace std;

double int_mc(double(*)(double), double&, double, double, int);
double f(double); 
void wait_for_key ();

int main()
{
    Gnuplot g("lines");

    double a = 0.0;              // left endpoint
    double b = 1.0;               // right endpoint
    int N = 500;
    
    double varest, mc;
    std::vector<double> x, err, var_e, var_t;

    for (int n=5; n <= N; n++)
    {
        mc = int_mc(f, varest, a, b, n);

        x.push_back(n);
        err.push_back(fabs(mc-1.0));
    }

    g.reset_plot();
    g.set_grid();
    g.set_style("lines").plot_xy(x,err,"erreur");

    wait_for_key();

    return 0;
}

/* 
    Fonction à intégrer 
*/
    double f(double x)
{
    double y = 0.0;
    return y;
}    

/* 
    Integration de Monte-Carlo
*/
double int_mc(double(*f)(double), double& varest, double a, double b, int n)
{
    return 0.0;
}

void wait_for_key ()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)  // every keypress registered, also arrow keys
    cout << endl << "Press any key to continue..." << endl;

    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    _getch();
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    cout << endl << "Press ENTER to continue..." << endl;

    std::cin.clear();
    std::cin.ignore(std::cin.rdbuf()->in_avail());
    std::cin.get();
#endif
    return;
}
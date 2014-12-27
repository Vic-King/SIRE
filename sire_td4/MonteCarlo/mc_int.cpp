#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "gnuplot_i.hpp"

using namespace std;

double int_mc(double(*)(double), double&, double&, double, double, int);
double f(double); 
void wait_for_key ();

int main()
{
    Gnuplot g("lines");

    double a = 0.0;              // left endpoint
    double b = 1.0;               // right endpoint
    int N = 500;
    
    double varest_t, varest_e, mc;
    std::vector<double> x, err, var_e, var_t;

    for (int n=5; n <= N; n++)
    {
        mc = int_mc(f, varest_t, varest_e, a, b, n);

        x.push_back(n);
        var_t.push_back(sqrt(varest_t));
        var_e.push_back(sqrt(varest_e));
        err.push_back(fabs(mc-1.0));

    }

    g.reset_plot();
    g.set_grid();
    g.set_style("lines").plot_xy(x,err,"erreur");
    //g.set_style("lines").plot_xy(x,var_t,"variance analytique");
    g.set_style("lines").plot_xy(x,var_e,"variance numerique");

    wait_for_key();

    return 0;
}

/* 
    Fonction à intégrer 
*/
double f(double x) {
    return 5.0f * x * x * x * x;
}    

/* 
    Integration de Monte-Carlo
*/
double int_mc(double(*f)(double), double& varest_t, double& varest_e, double a, double b, int n) {
    double resMc = 0.0f;
    double resVar_t = 0.0f;
    double resVar_e = 0.0f;

    double resF = 0.0f;

    srand (time(NULL));

    a *= 100.0f;
    b *= 100.0f;

    for (unsigned int i = 0; i < n; ++i) {
        double randf = (rand() % (int)(b - a) + (int)a) / 100.0f;

        resF = f(randf);

        resMc += resF;

        // variance analytique :
        resVar_t += (resF - 1) * (resF - 1);

        // variance numerique :
        resVar_e += resF * resF;
    }

    // variance analytique :
    varest_t = resVar_t / n;

    // variance numerique :
    varest_e = (1.0f / (n - 1)) * (resVar_e / n - (resMc / n) * (resMc / n));

    return resMc / n;
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

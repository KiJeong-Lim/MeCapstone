/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "header.hpp"

constexpr double ocvs[] =
{ 2.58503333333333 //   0%
, 2.90561016666667 //   2%
, 3.08249133333333 //   4%
, 3.17952500000000 //   6%
, 3.23710133333333 //   8%
, 3.28095000000000 //  10%
, 3.32413333333333 //  12%
, 3.36478333333333 //  14%
, 3.39867500000000 //  16%
, 3.42519616666667 //  18%
, 3.44861666666667 //  20%
, 3.47358333333333 //  22%
, 3.49503333333333 //  24%
, 3.51346083333333 //  26%
, 3.53090000000000 //  28%
, 3.55185000000000 //  30%
, 3.57477500000000 //  32%
, 3.59708333333333 //  34%
, 3.61538333333333 //  36%
, 3.63258333333333 //  38%
, 3.64896666666667 //  40%
, 3.66485000000000 //  42%
, 3.68090000000000 //  44%
, 3.69781666666667 //  46%
, 3.71550000000000 //  48%
, 3.73357500000000 //  50%
, 3.75171666666667 //  52%
, 3.76967500000000 //  54%
, 3.78728333333333 //  56%
, 3.80435000000000 //  58%
, 3.82127500000000 //  60%
, 3.83885000000000 //  62%
, 3.85778333333333 //  64%
, 3.87864166666667 //  66%
, 3.89880666666667 //  68%
, 3.91662500000000 //  70%
, 3.93353333333333 //  72%
, 3.95168333333333 //  74%
, 3.97141666666667 //  76%
, 3.99261666666667 //  78%
, 4.01393333333333 //  80%
, 4.03435833333333 //  82%
, 4.05259166666667 //  84%
, 4.06720000000000 //  86%
, 4.07875833333333 //  88%
, 4.08877250000000 //  90%
, 4.09895833333333 //  92%
, 4.11148333333333 //  94%
, 4.12865833333333 //  96%
, 4.15507500000000 //  98%
, 4.20280000000000 // 100%
};

AscMap const mySocOcvTable =
{ .ys = ocvs
, .left_bound_of_xs = 0.0
, .size_of_ys = LENGTH_OF(ocvs)
, .right_bound_of_xs = 100.0
};

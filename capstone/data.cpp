/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "capstone.hpp"

/* Q. What does the keyword `PROGMEM` do?
** A. See https://www.arduino.cc/reference/ko/language/variables/utilities/progmem/
*/

static constexpr
double const Ocvs[] PROGMEM =
{ 2.58503333333333
, 2.90561016666667
, 3.08249133333333
, 3.17952500000000
, 3.23710133333333
, 3.28095000000000
, 3.32413333333333
, 3.36478333333333
, 3.39867500000000
, 3.42519616666667
, 3.44861666666667
, 3.47358333333333
, 3.49503333333333
, 3.51346083333333
, 3.53090000000000
, 3.55185000000000
, 3.57477500000000
, 3.59708333333333
, 3.61538333333333
, 3.63258333333333
, 3.64896666666667
, 3.66485000000000
, 3.68090000000000
, 3.69781666666667
, 3.71550000000000
, 3.73357500000000
, 3.75171666666667
, 3.76967500000000
, 3.78728333333333
, 3.80435000000000
, 3.82127500000000
, 3.83885000000000
, 3.85778333333333
, 3.87864166666667
, 3.89880666666667
, 3.91662500000000
, 3.93353333333333
, 3.95168333333333
, 3.97141666666667
, 3.99261666666667
, 4.01393333333333
, 4.03435833333333
, 4.05259166666667
, 4.06720000000000
, 4.07875833333333
, 4.08877250000000
, 4.09895833333333
, 4.11148333333333
, 4.12865833333333
, 4.15507500000000
, 4.20280000000000
};

static constexpr
double const Vcells[] PROGMEM =
{ 2.66267511813557
, 2.97909231310534
, 3.15181171748037
, 3.24468356838222
, 3.29809792716228
, 3.33778453294892
, 3.37680578063660
, 3.41329372011026
, 3.44466601673130
, 3.47031048551871
, 3.49285426808954
, 3.51978808018582
, 3.54320525035293
, 3.56214602168726
, 3.57864450857180
, 3.59865380849759
, 3.62067556734825
, 3.64208066794380
, 3.66010221339239
, 3.67764853589568
, 3.69437819596763
, 3.70707134127572
, 3.71993114490973
, 3.73597886771615
, 3.75511455306103
, 3.77464191405084
, 3.79197083330198
, 3.80911640699331
, 3.82691894748863
, 3.84518682039352
, 3.86331301488147
, 3.88192541341650
, 3.90189614253335
, 3.92371543493947
, 3.94476496089448
, 3.96346782009276
, 3.98366306348376
, 4.00509997476904
, 4.02672013086287
, 4.04840685013661
, 4.07021024043437
, 4.09100964963846
, 4.10961739386516
, 4.12382483356325
, 4.13420693125392
, 4.14304485813845
, 4.15211217441638
, 4.16351864156142
, 4.17895109940276
, 4.20300118506630
};

AscList const mySocOcvTable =
{ .data_sheet_ref = &Ocvs
, .left_bound     = 0.00
, .right_bound    = 100.00
};

AscList const mySocVcellTable =
{ .data_sheet_ref = &Vcells
, .left_bound     = 0.00
, .right_bound    = 98.00
};

/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "header.hpp"

constexpr double ocvs[] = { 2.7, 3.0, 4.2 };

AscMap const mySocOcvTable =
{ .ys = ocvs
, .left_bound_of_xs = 0.0
, .size_of_ys = LENGTH_OF(ocvs)
, .right_bound_of_xs = 100.0
};

AscMap::AscMap(const double *const _ys, double const _left_bound_of_xs, size_t const _size_of_ys, double const _right_bound_of_xs)
  : left_bound_of_xs{ _left_bound_of_xs }
  , right_bound_of_xs{ _right_bound_of_xs }
  , ys{ _ys }
  , size_of_ys{ _size_of_ys }
{
}

AscMap::~AscMap()
{
}

double AscMap::calc_x(double const ratio) const
{
  return ((right_bound_of_xs - left_bound_of_xs) / (size_of_ys - 1) * ratio + left_bound_of_xs);
}

double AscMap::get_x(double const y) const
{
  double ratio = 0.0;
  int low = 0, high = size_of_ys - 1;

  while (low <= high)
  {
    int mid = low + ((high - low) / 2);

    if (ys[mid] > y)
    {
      high = mid - 1;
    }
    else if (ys[mid] < y)
    {
      low = mid + 1;
    }
    else
    {
      return calc_x((double)mid);
    }
  }
  return calc_x(((y - ys[high]) / (ys[low] - ys[high])) * (low - high) + high);
}

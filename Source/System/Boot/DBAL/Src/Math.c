/* Contains common maths related definitions.
* 
*  Copyright (c) 2011 Shikhin Sethi
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation;  either version 3 of the License, or
*  (at your option) any later version.
* 
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY// Without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
* 
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdint.h>
#include <Math.h>

// Finds out the sqrt of a floating-point value.
// double x                            The number whose square to find out.
//     rc
//                                     double - the square root of the number.
double sqrt(double x)
{
    double Result; 
    __asm__ __volatile__("fsqrt" : "=&t"(Result) : "f"(x));
    return Result;
}

// Finds out the log (base 2) of a floating-point value, and multiplies it with another number.
// double x                            The number whose log 2 to find out.
// double y                            The number to multiply to.
//     rc
//                                     double - the result.
double fyl2x(double x, double y)
{
    double Result; 
    __asm__ __volatile__("fyl2x" : "=&t"(Result) : "f"(x), "u"(y));
    return Result;
}

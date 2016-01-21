using System;
using System.Collections.Generic;
using System.Text;

namespace clrcore
{
    public class BinaryOperations
    {
        // Unsigned divition
        public static uint bin32udiv(uint a, uint b)
        {
            uint res = 0, counter, x, y;

            if (b == 0)
            {
                // TODO! Throw exception!
                return 0;
            }

            while (a >= b)
            {
                x = a >> 1;
                y = b;
                counter = 1;
                while (x >= y)
                {
                    y <<= 1;
                    counter <<= 1;
                }
                a -= y;
                res += counter;
            }
            return res;
        }

        // Unsigned module
        public static uint bin32urem(uint a, uint b)
        {
            return a - bin32udiv(a, b) * b;
        }

        public static int bin32div(int a, int b)
        {
            int negate = 0;
            if (a < 0)
            {
                negate = 1;
                a = -a;
            }

            if (b < 0)
            {
                negate^= 1;
                b = -b;
            }

            negate = -negate;
            negate |= ((negate >> 1) & 1) ^ 1;
            return negate * (int)bin32udiv((uint)a, (uint)b);
        }

        public static int bin32rem(int a, int b)
        {
            int negate = 0;
            if (a < 0)
            {
                negate = 1;
                a = -a;
            }

            if (b < 0)
            {
                negate ^= 1;
                b = -b;
            }

            negate = -negate;
            negate |= ((negate >> 1) & 1) ^ 1;
            return negate * (int)bin32urem((uint)a, (uint)b);
        }
    }
}

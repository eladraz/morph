using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    public class MathHelper
    {
        public static int SqrRoot(int x)
        {

            uint x1;
            int s, g0, g1;

            if (x <= 1) return x;
            s = 1;
            x1 = (uint)x - 1;
            if (x1 > 65535) { s = s + 8; x1 = x1 >> 16; }
            if (x1 > 255) { s = s + 4; x1 = x1 >> 8; }
            if (x1 > 15) { s = s + 2; x1 = x1 >> 4; }
            if (x1 > 3) { s = s + 1; }

            g0 = 1 << s;                // g0 = 2**s.
            g1 = (g0 + (x >> s)) >> 1;  // g1 = (g0 + x/g0)/2.

            while (g1 < g0)
            {           // Do while approximations
                g0 = g1;                 // strictly decrease.
                g1 = (g0 + (x / g0)) >> 1;
            }
            return g0;
        }
        public class Trigonometry
        {
            private static int[] sinTable = { 0, 17, 34, 52, 69, 87, 104, 121, 139, 156, 173, 190, 207, 224, 241, 258, 275, 292, 309, 325, 342, 358, 374, 390, 406, 422, 438, 453, 469, 484, 499, 515, 529, 544, 559, 573, 587, 601, 615, 629, 642, 656, 669, 681, 694, 707, 719, 731, 743, 754, 766, 777, 788, 798, 809, 819, 829, 838, 848, 857, 866, 874, 882, 891, 898, 906, 913, 920, 927, 933, 939, 945, 951, 956, 961, 965, 970, 974, 978, 981, 984, 987, 990, 992, 994, 996, 997, 998, 999, 999 };

            public static int Sin(int angle, int multiply)
            {
                if (angle >= 90 || angle <= 0)
                {
                    return -1;
                }
                else
                {
                    return sinTable[angle] * multiply / 1000;
                }
            }

            public static int Cos(int angle, int multiply)
            {
                if (angle >= 90 || angle <= 0)
                {
                    return -1;
                }
                else
                {
                    return (sinTable[90-angle]) * multiply / 1000;
                }
            }

        }
    }
}

using System;

namespace Morph
{
    public class Math
    {
        public static byte Max (byte val1, byte val2)
        {
            if (val1 > val2 )
            {
                return val1;
            }
            return val2;
        }

        public static int Min(int val1, int val2)
        {
            if (val1 < val2)
            {
                return val1;
            }
            return val2;
        }

        public static int Max(int val1, int val2)
        {
            if (val1 > val2)
            {
                return val1;
            }
            return val2;
        }

        public static int Abs(int value)
        {
            if (value == Int32.MinValue)
                throw new System.OverflowException("Value is too small.");
            return (value < 0) ? -value : value;
        }

        public static long Abs(long value)
        {
            if (value == Int64.MinValue)
                throw new System.OverflowException("Value is too small.");
            return (value < 0) ? -value : value;
        }



        public static short Abs(short value)
        {
            if (value == Int16.MinValue)
                throw new System.OverflowException("Value is too small.");
            return (short)((value < 0) ? -value : value);
        }

    }
}

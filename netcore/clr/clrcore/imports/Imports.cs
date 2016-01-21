namespace Morph
{
    public class Imports
    {
        // Implement in C# as inline
        [clrcore.CompilerOpcodes("nop")]
        public static System.Array convertToArray(Morph.Array o)
        {
            return (System.Array)(object)(o);
        }

        [clrcore.CompilerOpcodes("nop")]
        public static Morph.String convertToMorphString(string o)
        {
            return (Morph.String)(object)(o);
        }

        // Implement in C# as inline
        [clrcore.CompilerOpcodes("nop")]
        public static string convertToString(System.Object o)
        {
            return (string)(o);
        }

        // Implement in C# as inline
        [clrcore.CompilerOpcodes("nop")]
        public static object convertToObject(System.String o)
        {
            return (object)(o);
        }

        // Convert System.object to direct pointer
        // Compiler intferace does nothing, and implict convertion will be invoked
        [clrcore.CompilerOpcodes("nop")]
        public unsafe static void* convert(System.Object o)
        {
            return null;
        }

        [clrcore.Import("_malloc"), clrcore.CallingConvention("cdecl")]
        public unsafe static void* allocate(uint size)
        {
            return null;
        }

        [clrcore.Import("_free"), clrcore.CallingConvention("cdecl")]
        public unsafe static void free(void* mem)
        {
        }

        [clrcore.Import("_callFunction"), clrcore.CallingConvention("cdecl")]
        public unsafe static void callFunction(void* handler, void* param)
        {
        }

        [clrcore.Import("_debugChar"), clrcore.CallingConvention("cdecl")]
        public static void debugChar(char ch)
        {
        }

        [clrcore.Import("_assertFalse"), clrcore.CallingConvention("cdecl")]
        public static void assertFalse()
        {
        }

        [clrcore.Import("_unhandledException"), clrcore.CallingConvention("cdecl")]
        public static void unhandledException(System.Object exceptionObject)
        {
        }


        [clrcore.Import("_getTickCount"), clrcore.CallingConvention("cdecl")]
        public static int getTickCount()
        {
            return 12345;
        }

        [clrcore.Import("_getNumberOfTicksInMillisecond"), clrcore.CallingConvention("cdecl")]
        public static int getNumberOfTicksInMillisecond()
        {
            return 1000;
        }

        //[clrcore.CompilerOpcodes("mul32h")]
        //public static uint mul32high(out uint result, uint a1, uint a2)
        //{
        //    ulong m = (ulong)a1 * a2;
        //    result = (uint)(m & 0xFFFFFFFF);
        //    return (uint)(m >> 32);
        //}

        //[clrcore.CompilerOpcodes("add32_carry")]
        //public static uint add32ReturnCarry(out uint result, uint a1, uint a2)
        //{
        //    ulong m = (ulong)a1 + a2;
        //    result = (uint)(m & 0xFFFFFFFF);
        //    return (uint)(m >> 32);
        //}


        ///*
        // *  resultHigh:resultLow = high1:low1 + high2:low2
        // *
        // *  resultLow            = low1+low2
        // *  resultHighWithCarry  = carry+high1+high2
        // */
        //[clrcore.CompilerOpcodes("adc32")]
        //public static void add32WithCarry(out uint resultHigh, out uint resultLow, uint high1, uint high2, uint low1, uint low2)
        //{
        //    uint carry = Imports.add32ReturnCarry(out resultLow, low1, low2);
        //    resultHigh = carry + high1 + high2;
        //}

        //[clrcore.CompilerOpcodes("sub32_carry")]
        //public static uint sub32ReturnBorrow(out uint result, uint a1, uint a2)
        //{
        //    ulong m = (ulong)a1 - a2;
        //    result = (uint)(m & 0xFFFFFFFF);
        //    return (uint)(m >> 63);
        //}

        ///*
        // *  resultHigh:resultLow = high1:low1 - high2:low2
        // *
        // *  resultLow            = low1-low2
        // *  resultHighWithCarry  = high1-high2-carry
        // */
        //[clrcore.CompilerOpcodes("sbb32")]
        //public static void sub32WithBorrow(out uint resultHigh, out uint resultLow, uint high1, uint high2, uint low1, uint low2)
        //{
        //    uint borrow = Imports.sub32ReturnBorrow(out resultLow, low1, low2);
        //    resultHigh = high1 - high2 - borrow;
        //}
    }
}

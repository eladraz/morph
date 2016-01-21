using System;

namespace TestSimpleTypes1
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Test
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
        /*
         * Assumes CLR supports:
         *    - Native types (int*, uint*, string)
         *    - Native binary/unary operations
         *    - Copying and changing context
         *    - Arraies
         *    - System.Object API
         *    - System.Console.Write*
         *    - System.String API (=string)
         *
         * CLR will not be tested for:
         *    - Pointers        (See TestSimpleTypes2)
         *    - Exceptions      (See TestSimpleTypes2)
         *    - References      (See TestSimpleTypes3)
         *    - Classes         (See TestSimpleTypes3)
         *    - ValueTypes      (See TestSimpleTypes3)
         *    - Corlib          (See TestSimpleTypes4)
         *
         *    - Floating points
         *          (Which is not implemented in ring0 only as emulator)
         */
		public static void Main(string[] args)
		{
            System.Console.WriteLine("Hello eric!");
            System.Console.WriteLine("TestSimpleTypes1::Test::Main()");
            System.Console.WriteLine();

            System.Console.WriteLine("Executing first test: Native Type test...");
            testCopying();

            System.Console.WriteLine("Executing second test: boxed objects...");
            testNativeBoxedObjects();


            System.Console.WriteLine();
            System.Console.WriteLine();
            System.Console.WriteLine("Tests: OK!");
		}

        // Static variable with initializer
        public static int m6 = 6;

        /*
         * Transform a number into binary string
         */
        static unsafe string printBinary(uint number)
        {
            string ret = "";
            uint numberOfBits = sizeof(uint) * 8;
            for (uint i = 0; i < numberOfBits; i++)
            {
                if ((number & 1) == 1)
                    ret = "1" + ret;
                else
                    ret = "0" + ret;

                number = number >> 1;
            }

            return ret;
        }

        /*
         * Test:
         * Binary operators:
         *    - Arithmetic +   -   *   /   %
         *    - Logical (boolean and bitwise) &   |   ^   !   ~   &&   ||
         *               true   false
         *    - Relational ==   !=   <   >   <=   >=
         *    - Shift <<   >>
         * Unary operators:
         *    - Increment, decrement ++   --
         *
         * Assignment =   +=   -=   *=   /=   %=   &=   |=   ^=   <<=   >>=
         */
        static void testCopying()
        {
            // Local stack: Define the default system integer types
            System.Byte  uint8;      System.SByte int8;
            System.UInt16 uint16;    System.Int16 int16;
            System.UInt16 uint16_2;
            System.UInt32 uint32;    System.Int32 int32;
            System.UInt64 uint64;    System.Int64 int64;
            bool boolean;

            uint16 = 6655;
            uint16_2 = uint16;
            uint16 = 7788;
            if (uint16_2 != 6655)
            {
                System.Console.WriteLine("   ERROR Copy-on-write " + uint16_2 + uint16);
            }
            else
            {
                System.Console.WriteLine("   PASS " + uint16 + " " + uint16_2 +
                    "  =  7788 6655");
            }

            uint8 = (byte)m6;               // operator = (static)
            int8  = 2;                      // operator = (immediate)
            int64 = -13;
            int64+= (m6 * 3);               // operator *
            int64++;
            int32 = int8;                   // operator = (local)
            int32*= 3;

            if (int32 != int64) // Test operator != and block jump
            {
                System.Console.Write("   ERROR1 ");
                System.Console.WriteLine("(" + int32 + " != " + int64 + ")");
            }

            if (int64 == m6)
            {
                System.Console.Write("   PASS ");
                System.Console.WriteLine("(" + int32 + " == " + int64 + " == " + m6 + ")");
            } else
            {
                System.Console.Write("   ERROR2 ");
                System.Console.WriteLine("(" + int32 + " != " + int64 + ")");
            }

            // Test operator <, > signed and unsigned
            int16 = -5;
            uint16 = (System.UInt16)int16;
            if (int16 < 0)
                System.Console.WriteLine("   PASS (" + int16 + "  (unsigned)  " + uint16 + ")");
            if (uint16 < 0)
                System.Console.WriteLine("   ERROR3 ");

            // Signed / unsigned convert
            if (int16 == uint16)
                System.Console.WriteLine("   ERROR4 ");
            if (int16 >= uint16)
                System.Console.WriteLine("   ERROR5 ");
            else
                System.Console.WriteLine("   PASS (signed.unsigned)");

            // Test boolean operations
            boolean = true;
            boolean = boolean && false;
            if (boolean)
                System.Console.WriteLine("   ERROR6");
            else
                System.Console.WriteLine("   PASS");

            boolean = boolean || true;
            if (boolean)
                System.Console.WriteLine("   PASS");
            else
                System.Console.WriteLine("   ERROR7");

            if (!boolean)
                System.Console.WriteLine("   ERROR8");
            else
                System.Console.WriteLine("   PASS");

            // Test bitwise operation
            uint32 = 0xAA;
            System.Console.WriteLine("   PASS (" + printBinary(uint32) + ")");
            uint32 = ~uint32;
            System.Console.WriteLine("   PASS (" + printBinary(uint32) + ")");

            uint64 = uint32;
            if (System.Object.ReferenceEquals(uint64, uint32))
                System.Console.WriteLine("   ERROR10");
            else
                System.Console.WriteLine("   PASS");

            printBinaryFormat(1 +
                0x100 +
                0x102 +
                0x1000 +
                0x1001 +
                0x1002 +
                0xF000000,
                5);
        }

        /*
         * Recursive function which print a 2D triangle picture from a number
         */
        static void printBinaryFormat(uint num, int itr)
        {
            if (itr <= 0)
                return;

            System.Console.WriteLine(printBinary(num));
            printBinaryFormat((num << 1) | num, itr - 1);
            System.Console.WriteLine(printBinary(num));
        }

        /*
         *
         */
        static unsafe void testNativeBoxedObjects()
        {
            // Local stack: Define the default system integer types
            System.Byte  uint8;      System.SByte  int8;
            System.UInt16 uint16;    System.Int16 int16;
            System.UInt32 uint32;    System.Int32 int32;
            System.UInt64 uint64;    System.Int64 int64;
            char character;
            bool boolean;

            uint8 = 255;
            int8 = (System.SByte)uint8; // signed to unsigned cast
            if ((int8.ToString() == "-1") &&
                (uint8.ToString() == "255") &&
                (int8.GetType().ToString() == "System.SByte") &&
                (uint8.GetType().ToString() != "System.SByte") &&
                (uint8.GetType().ToString() == "System.Byte"))
            {
                System.Console.WriteLine("   PASS   [" +
                    uint8.GetType().ToString() + ", " +
                    int8.GetType().ToString() + "]");
            } else
            {
                System.Console.WriteLine("   ERROR1");
                System.Console.WriteLine("   Repeating tests...");
                if ((int8.ToString() != "-1")) {
                    System.Console.WriteLine("     int8.ToString(): ", int8.ToString());
                }
                if ((uint8.ToString() != "255"))
                {
                    System.Console.WriteLine("     uint8.ToString(): ", uint8.ToString());
                }

                System.Console.WriteLine("     ", int8.GetType().ToString());
                System.Console.WriteLine("     ", uint8.GetType().ToString());
            }
        }

        /*
         * String concatenation +
         * Member access .
         * Indexing []
         * Cast ()
         * Conditional ?:
         * Delegate concatenation and removal +   -
         * Object creation new
         * Type information is   sizeof   typeof
         * Overflow exception control: checked   unchecked
         * Indirection and Address *   ->   []   &
         */
	}
}

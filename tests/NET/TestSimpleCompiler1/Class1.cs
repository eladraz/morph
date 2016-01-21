using System;

namespace TestSimpleCompiler1
{
    class DummyClass
    {
        public DummyClass()
        {
            m_member = 0;
            m_member1 = 0;
        }

        public int m_member;
        public int m_member1;
    }

	class Class1
	{
		public enum CarOptions
		{
			SunRoof = 0x01,
			Spoiler = 0x02,
			FogLights = 0x04,
			TintedWindows = 0x08,
		}

        [clrcore.Export("_fibonacci")]
        static int fib(int x)
        {
            if (x < 2)
                return 1;
            return fib(x-1) + fib(x-2);
        }

        static char toUpper(char ch)
        {
            if ((ch >= 'a') && (ch <= 'z'))
            {
                ch = (char)(ch - 'a' + 'A');
            }

            return ch;
        }
        /**/

        /**/
        static char testIntegerOperations()
        {
            // Basic types
            System.UInt16 u16;
            System.UInt32 u32;
            u16 = 4;
            u16+= 5;
            u32 = 100;
            u32/= u16;

            // Bytes chars and ASCII
            System.Byte u8;
            System.Char c = 'a';
            System.SByte i8;
            u8 = (System.Byte)c;
            i8 = (System.SByte)c;
            u16 = (System.UInt16)((i8/u8) + c);

            if (u32 == 0)
            {
                // wrong
                u32 = 0xDEADFACE;
            }
            else
            {
                u32 = 0x777;
            }
            // return (int)u32;

            // Branch
            System.Int32 i32 = 5;
            i32+= 100;
            // First branch
            if (i32 == 105)
            {
                i32 = 0;
            }
            else
            {
                i32 = -1;
            }

            // ia32 is zero
            i32 = i32 * (5 * (20 - 4));

            // Second branch
            if (i32 != 0)
            {
                i32-= 1;
            }
            else
            {
                i32 = 0;
            }
            // i32 must be zero
            if (i32 != 0)
                return 'B';

            byte a = byte.MaxValue, b = byte.MinValue;

            Console.WriteLine("byte.MaxValue = " + a);
            Console.WriteLine("byte.MaValue = " + b);

            bool A = false;
            bool B = true;

            A = A && B;

            Console.WriteLine("A is " + A);

            // Check all other method
            // Test for uppercase
            return toUpper('f');
        }

        unsafe static bool testIntArries()
        {
            bool ret = false;
            int[] ar = new int[10]; // Morph.Imports.allocate(10 * sizeof(int));
            int i = 0;
            for (i = 0; i < 10; i++)
            {
                ar[i] = fib(i + 3);
            }
            ret = true;
            for (i = 2; i < 10; i++)
            {
                if ((ar[i - 2] + ar[i - 1]) != ar[i])
                {
                    ret = false;
                }
            }

            int size = ar.Length;
            if (size == 10)
                Console.Write("Array Length OK! " + size + "  ");
            else
                Console.Write("Array Length FAILED" + size + "!!!  ");

            for (i = 0; i < 10; i++)
            {
                Console.Write(ar[i]);
                Console.Write(' ');
            }
            return ret;
        }

        unsafe static bool testObjectArries()
        {
            bool ret = false;
            DummyClass[] ar = new DummyClass[10]; // Morph.Imports.allocate(10 * sizeof(int));
            int i = 0;
            for (i = 0; i < 10; i++)
            {
                ar[i] = new DummyClass();
            }

            ret = true;
            for (i = 2; i < 10; i++)
            {
                ar[i] = null;
            }

            int size = ar.Length;
            if (size == 10)
                Console.Write("Array Length OK! " + size + "  ");
            else
                Console.Write("Array Length FAILED" + size + "!!!  ");

            for (i = 0; i < 10; i++)
            {
                Console.Write(ar[i]);
                Console.Write(' ');
            }
            return ret;
        }

        unsafe static bool testArraies()
        {
            return testIntArries();// && testObjectArries();
        }

        /**/

        /*
         * The most simplest struct. Contains only three integers
         */
        public struct Elements
        {
            public byte  m_int8;
            public short m_int16;
            public uint  m_int32;

            public static uint Magic = 42;
        }

        /**/
        static bool testStructures()
        {
            bool ret = true;
            Elements element;
            element.m_int32 = (uint)0xDEADFACE;
            element.m_int16 = 0x1010;
            element.m_int8 = 8;
            Elements obj = element;

            if ((obj.m_int16 != 0x1010) ||
                (obj.m_int32 < 3150) ||
                (obj.m_int8 > 8))
                ret = false;

            Elements copy = obj;
            obj.m_int32 = 42;
            if (obj.m_int32 == Elements.Magic)
            {
                obj.m_int32 = Elements.Magic;
            }
            else
            {
                ret = false;
            }

            obj.m_int16++;

            if ((copy.m_int16 == 0x1010) &&
                (obj.m_int16 == 0x1011))
            {
                copy.m_int16 = 0;
            }
            else
            {
                ret = false;
            }

            if (copy.m_int32 == Elements.Magic)
            {
                ret = false;
            }
            Elements.Magic = 0x666;
            copy.m_int32 = 0x42;
            if (copy.m_int32 == Elements.Magic)
            {
                ret = false;
            }
            copy.m_int32 = Elements.Magic;
            if (0x666 != Elements.Magic)
            {
                ret = false;
            }
            if (copy.m_int32 != Elements.Magic)
            {
                ret = false;
            }

            return ret;
        }

        static bool testIncOperator()
        {
            int[] numbers = new int[10];
            int i;
            for (i = 0; i < 10; i++)
            {
                if (numbers[i] != 0)
                    return false;
            }

            i = 3;
            numbers[i++] = 5;

            if (i != 4)
                return false;
            if (numbers[4] != 0)
                return false;
            if (numbers[3] != 5)
                return false;

            i = 6;
            numbers[++i] = 5;
            if (i != 7)
                return false;
            if (numbers[6] != 0)
                return false;
            if (numbers[7] != 5)
                return false;

            return true;
        }

        [clrcore.Export("_myMain"), clrcore.CallingConvention("cdecl")]
		static int Main()
		{
            bool isError = false;
            int sucessCount = 0;
            int errorCount = 0;

            Console.WriteLine("(C) Elad Raz");
            Console.WriteLine("Integrity-Project: Start testing....");

            Console.Write("Testing Fibonacci...    ");
            isError = isError | (fib(0) != 1);
            isError = isError | (fib(1) != 1);
            isError = isError | (fib(2) != 2);
            isError = isError | (fib(3) != 3);
            isError = isError | (fib(4) != 5);
            isError = isError | (fib(5) != 8);
            isError = isError | (fib(6) != 13);

            if (fib(6) == 13)
            {
                sucessCount++;
            }
            else
            {
                errorCount++;
                isError = true;
            }

            if (isError)
            {
                Console.WriteLine("FAILED");
            }
            else
            {
                Console.WriteLine("OK");
            }

            Console.Write("Testing Inc operator...    ");
            if (testIncOperator())
                Console.WriteLine("OK!");
            else
                Console.WriteLine("FALSE!");

            Console.WriteLine("Testing booleans...      ");
            bool t = true;
            bool f = false;
            Console.WriteLine("  True  = " + t.ToString());
            Console.WriteLine("  False = " + f.ToString());

            Console.Write("Testing Integers...     ");
            if (testIntegerOperations() == 'F')
            {
                sucessCount++;
                Console.WriteLine("OK");
            }
            else
            {
                errorCount++;
                isError = true;
                Console.WriteLine("FAILED");
            }

            Console.WriteLine("MulTable:");
            for (int i = 1; i <= 10; i++)
            {
                for (int j = 1; j <= 10; j++)
                {
                    Console.Write(i * j);
                    Console.Write("  ");
                }
                Console.WriteLine("");
            }

            Console.Write("Testing arraies...      ");
            if (testArraies())
            {
                sucessCount++;
                Console.WriteLine("OK");
            }
            else
            {
                errorCount++;
                isError = true;
                Console.WriteLine("FAILED");
            }

            Console.Write("Testing structs...      ");
            if (testStructures())
            {
                sucessCount++;
                Console.WriteLine("OK");
            }
            else
            {
                errorCount++;
                isError = true;
                Console.WriteLine("FAILED");
            }

            if (isError)
            {
                errorCount++;
                Console.WriteLine();
                Console.WriteLine("Tests failed with some errors");
            }
            else
            {
                Console.WriteLine();
                Console.WriteLine("Tests completed successfully!");
                Console.Write("Number of tests runs: ");
                Console.Write(sucessCount);
                Console.Write("   failed: ");
                Console.Write(errorCount);
                Console.WriteLine();
            }

            return (errorCount << 8) | sucessCount;
        }
	}
}

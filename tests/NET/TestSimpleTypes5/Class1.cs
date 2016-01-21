using System;
using TestSimpleTypes3;

namespace TestSimpleTypes5
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	/// Author: Elad Raz
	class Class1
	{
		/// <summary>Execute tests</summary>
		static void Main(string[] args)
		{
            System.Console.WriteLine("Executing tests...");
            test();
            System.Console.WriteLine();
            System.Console.WriteLine();
            System.Console.WriteLine("OK!");
		}

        static void test()
        {
            // Test null assignment
            Humman mine = null;
            if (mine == null)
            {
                System.Console.WriteLine("  PASS");
            } else
            {
                System.Console.WriteLine("  ERROR1 ");
            }

            // Trying to access null method
            try
            {
                mine.eat();
                System.Console.WriteLine("  ERROR2 ");
            }
            catch (NullReferenceException e)
            {
                System.Console.WriteLine("  PASS: " + e.ToString());
            }
            catch (Exception e)
            {
                System.Console.WriteLine("  PASS: SOFAR: " + e.ToString());
            }


            // Value-type and non null assignment
            GPS a = new GPS(1,2,3);
            GPS b = a;
            a.m_y = 3;
            if ((b.m_y == 2) &&
                (a.m_y == 3))
            {
                System.Console.WriteLine("  PASS ");
            } else
            {
                System.Console.WriteLine("  ERROR3 ");
            }

            // Tets non assigned value-types
            GPS c;
            c.m_x = 9;
            c.m_y = 10;
            c.m_z = 11;
            if ((c.m_x == 9) &&
                (c.m_y == 10) &&
                (c.m_z == 11))
            {
                System.Console.WriteLine("  PASS ");
            } else
            {
                System.Console.WriteLine("  ERROR4 ");
            }

            string str = null;
            string ustr = "bla";
            if ((str == null) &&
                (ustr != null))
            {
                System.Console.WriteLine("  PASS ");
            } else
            {
                System.Console.WriteLine("  ERROR6 ");
            }
        }
	}
}

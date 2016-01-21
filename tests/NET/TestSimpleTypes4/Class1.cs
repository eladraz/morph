using System;

namespace TestSimpleTypes4
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
        class DestructorTest {
            public static uint gdCount = 0;
            public DestructorTest() {
                gdCount++;
                System.Console.WriteLine("DestructorTest generated " + gdCount);
            }

            ~DestructorTest() {
                gdCount--;
                System.Console.WriteLine("DestructorTest destroied OK! " + gdCount);
            }
        }
		/// <summary>
		/// The main entry point for the application.
		/// This routine should test exception-handling capabilities
		/// The class assumes Native-Types, Pointers, Classes are working in
		/// order
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
            System.Console.WriteLine("Exception handling test...");
            test1();
            test2();
            test3();
            test4();
            test5();
            System.Console.WriteLine("OK!");
		}

        static void test1()
        {
            System.Console.WriteLine("Execute test1()...");
            try
            {
                int x = 0;
                int y = 15 / x;
            }
            catch (System.ArgumentOutOfRangeException)
            {
                System.Console.WriteLine("  ERROR! Wrong exception handler");
            }
            catch (System.Exception exception)
            {
                System.Console.WriteLine("  PASS! Exception throwed..");
                System.Console.WriteLine("Exception: " +
                                        exception.ToString().Substring(0, 60));
                return;
            }
            // This code should never executed
            try
            {
                System.Console.WriteLine("  ERROR! No exception thrown!");
            }
            catch (System.OverflowException ovl)
            {
                System.Console.WriteLine("  ERROR! Overflow exception. ");
            }
        }

        static void test2()
        {
            try
            {
                System.Console.WriteLine("Execute test2()...");
                throw(new Exception());
                System.Console.WriteLine("  ERROR!! test2().");
            }
            catch (System.Exception e)
            {
                System.Console.WriteLine("  test2(). OK!");
                return;
            }
            System.Console.WriteLine("  ERROR! test2().");
        }

        static void test3()
        {
            try
            {
                System.Console.WriteLine("Execute test3()...");
                innerThrowException();
                System.Console.WriteLine("  ERROR!! test3().");
            }
            catch (System.Exception e)
            {
                System.Console.WriteLine("  test3(). OK!");
            }
        }

        static void test4()
        {
            int i = 123;
            string s = "Some string";
            object o = s;

            try
            {
                DestructorTest object1 = new DestructorTest();
                System.Console.WriteLine("Execute test4()...");
                try
                {
                    // Illegal conversion; o contains a string not an int
                    DestructorTest object2 = new DestructorTest();
                    i = (int) o;
                }
                catch (System.Exception e)
                {
                    System.Console.WriteLine("   exception caught.");
                    System.Console.WriteLine("   Rethrow");
                    throw;
                }
            }
            catch (System.Exception e)
            {
                System.Console.WriteLine("   test4() OK.");
            }
        }

        static void test5()
        {
            try
            {
                innerThrowZeroException();
                System.Console.WriteLine("  ERROR!! test5(). not thrown");
            }
            catch (System.DivideByZeroException)
            {
                System.Console.WriteLine("   test5() OK.");
            }
            catch (System.Exception)
            {
                System.Console.WriteLine("   test5() Wrong exception handler.");
            }
        }

        static void innerThrowZeroException()
        {
            try
            {
                // Throw exception
                innerThrowException();
                System.Console.WriteLine("  ERROR!! innerThrowZeroException(). not thrown");
            }
            catch (System.Exception)
            {
                int a = 0;
                // Explicit throw
                int b = 5 / a;
            }
        }

        static void innerThrowException()
        {
            DestructorTest objectNone;
            DestructorTest object1 = new DestructorTest();
            DestructorTest object2 = new DestructorTest();
            // Just throw exception
            throw(new Exception());
        }
	}
}

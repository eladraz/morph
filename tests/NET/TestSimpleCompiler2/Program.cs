using System;
using System.Collections;

namespace TestSimpleCompiler2
{
    class A
    {
        public A(int memb)
        {
            m_member = memb;
            m_member1 = 3;
            Console.WriteLine("A::A() has been called     m_member = " + m_member);
        }

        ~A()
        {
            Console.WriteLine("A::~A() has been called     m_member = " + m_member);
        }

        public override int GetHashCode()
        {
            return m_member + m_member1;
        }

        public int Add(int x, int y)
        {
            return x + y + m_member + m_member1;
        }

        public int m_member;
        public int m_member1;
        public ImplementationClass h;
    }

    /////////////////////////////////////////////////////////////////////
    // Taken from MSDN
    /*
    public class Stack<T>
    {
        //readonly int m_Size;
        //int m_StackPointer = 0;
        // TODO! T[] m_Items;
        T m_item;
        public Stack()
         //   : this(100)
        { }

        //public Stack(int size)
        //{
        //    //m_Size = size;
        //    //m_Items = new T[m_Size];
        //}


        public void Push(T item)
        {
            //if (m_StackPointer >= m_Size)
            //    throw new StackOverflowException();
            // TODO! m_Items[m_StackPointer] = item;
            // m_StackPointer++;


            m_item = item;
        }
        public T Pop()
        {
            /*
            m_StackPointer--;
            if (m_StackPointer >= 0)
            {
                return m_Items[m_StackPointer];
            }
            else
            {
                m_StackPointer = 0;
                //throw new InvalidOperationException("Cannot pop an empty stack");
                return m_Items[m_StackPointer]; // TODO!
            }
            return m_item;
        }
    }
     */

    interface ISampleInterface
    {
        // Increase reference
        void Inc();
    }

    interface IPwerInterface : ISampleInterface
    {
        void DoubleInc();
    }

    class ImplementationClass : IPwerInterface
    {
        public ImplementationClass()
        {
            m_counter = 0;
            Console.WriteLine("ImplementationClass() called, counter = " + m_counter);
        }

        ~ImplementationClass()
        {
            Console.WriteLine("~ImplementationClass() has been called, counter = " + m_counter);
        }

        public override int GetHashCode()
        {
            return m_counter;
        }

        // Explicit interface member implementation:
        void ISampleInterface.Inc()
        {
            // Method implementation.
            m_counter++;
        }

        void IPwerInterface.DoubleInc()
        {
            m_counter += 2;
        }

        private int m_counter;
    }
    /////////////////////////////////////////////////////////////////////


    class Program
    {
        static void testSingleLocal(bool bInit)
        {
            Object o;
            if (bInit)
                o = new A(10);
        }
        static void testTwoLocals(int pass)
        {
            Object o, p;
            if ((pass & 1) != 0)
                o = new A(pass * 100 + 1);
            if ((pass & 2) != 0)
                p = new A(pass * 100 + 2);
        }
        static void testManyLocals(int pass)
        {
            int before = 1;
            Object o, p;
            int middle = 2;
            Object q, r;
            int after = 3;

            before = middle * after;

            if ((pass & 1) != 0)
                o = new A(pass * 1000 + 3);
            if ((pass & 2) != 0)
                p = new A(pass * 1000 + 4);

            middle = before * after;

            if ((pass & 4) != 0)
                q = new A(pass * 1000 + 5);
            if ((pass & 8) != 0)
                r = new A(pass * 1000 + 6);

            after = before * middle;
        }

        static void testLocalsCleanup()
        {
            Console.WriteLine("Testing locals cleanup...");
            testSingleLocal(false);
            testSingleLocal(true);

            for (int pass = 0; pass < 4; pass++)
                testTwoLocals(pass);

            for (int pass = 0; pass < 16; pass++)
                testManyLocals(pass);
        }

        static void testTypedef(System.Object il)
        {
            if (il is IPwerInterface)
                Console.WriteLine("OK il is IPwerInterface");
            if (il is ImplementationClass)
                Console.WriteLine("OK il is ImplementationClass");
            if (il is System.Object)
                Console.WriteLine("OK il is System.Object");
            if (il is System.ValueType)
                Console.WriteLine("NOK!");
            if (il is ISampleInterface)
                Console.WriteLine("OK il is ISampleInterface");
        }

        static void testBoxing()
        {
            Console.WriteLine("Testing boxing...");
            int g = 234;

            Console.WriteLine("From Int:    " + g);
            object o = g;
            Console.WriteLine("From Object: " + o);
            g = 123;
            Console.WriteLine("From Object: " + o);
            int j = (int)o;
            j--;
            Console.WriteLine("From Int:    " + j);
            Console.WriteLine("From Object: " + o);
        }

        static void testInterfaces()
        {
            Console.WriteLine("Starting interface test...");
            ImplementationClass il = new ImplementationClass();
            ISampleInterface counter = il;

            for (int i = 0; i < 66; i++)
                counter.Inc();
            Console.WriteLine("Counter [66] = " + il.GetHashCode());
            IPwerInterface pw = (IPwerInterface)(counter);
            for (int i = 0; i < 66; i++)
                pw.Inc();
            Console.WriteLine("Counter [132] = " + il.GetHashCode());
            for (int i = 0; i < 66; i++)
                pw.DoubleInc();
            Console.WriteLine("Counter [264] = " + il.GetHashCode());

            testTypedef(counter);
        }

        interface IsomeInterface
        {
            int foo1(byte[] output, int outOff);

        }
        class MyVirtual : IsomeInterface
        {
            public virtual int foo1(byte[] output, int outOff)
            {
                Console.WriteLine("bad:foo1");
                return 0;
            }
        }

        class MySon : MyVirtual
        {
            public override int foo1(byte[] output, int outOff)
            {
                Console.WriteLine("good:foo1");
                return 1;
            }
        }
        static void testInterfaces1()
        {
            MySon a = new MySon();
            byte[] arr = new byte[10];
            a.foo1(arr, 10);
        }

        static string getLongString()
        {
            return "This is Ground Control to Major Tom\nYou've really made the grade\nAnd the papers want to know whose shirts you wear\nNow it's time to leave the capsule if you dare";
        }

        static void testLongStrings()
        {
            Console.WriteLine("Return getLongString() = " + getLongString());
        }

        struct MyStruct
        {
            public int v1;
        }

        static void testHashCode()
        {
            MyStruct a = new MyStruct();
            a.v1 = 5;
            Console.WriteLine(a.GetHashCode());
        }

        static void testClassed()
        {
            Console.WriteLine("Integrity-Project: Start class testing....");
            A a = new A(0);
            A b = a;
            b.m_member = 66;

            Console.WriteLine("B.m_member                   = " + b.m_member);
            Console.WriteLine("A.add(22,12) [Should be 100] = " + a.Add(22, 9));
            Console.WriteLine("A.GetHasCode()               = " + a.GetHashCode());
            object o = b;
            Console.WriteLine("Object.GetHasCode()          = " + o.GetHashCode());
        }

        static void testFieldRVA()
        {
            char[] chars = new char[] {'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
            for (int i = 0; i < chars.Length; i++)
            {
                Console.Write(chars[i]);
            }
            Console.WriteLine();
            int[] ints = new int[] { 3,1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,
                                     3,3,8,3,2,7,9,5,0,2,8,8,4,1,9,7,1,6,9,3,9,9,3,7 };
            for (int i = 0; i < ints.Length; i++)
            {
                Console.Write(ints[i]);
            }
            Console.WriteLine();
        }

        static void testLocalsOverwrite()
        {
            A a = new A(0);
            A b = new A(12);
            a = new A(5);
            b = null;
            a = new A(10);
        }

        struct ObjectContainers
        {
            public A m_member1;
            public A m_member2;
        }

        static void testLocalsOverwrite1()
        {
            ObjectContainers a = new ObjectContainers();
            a.m_member1 = new A(0);
            a.m_member1 = null;
            a.m_member2 = new A(5);
            a.m_member1 = a.m_member2;
            a.m_member2 = null;
        }

        static void testGenerics()
        {
            //Stack<int> test = new Stack<int>();
            //Stack<byte> test1 = new Stack<byte>();
            //int t;
            //test.Push(13);
            //test1.Push(12);
            //t = test.Pop();
            //test1.Pop();
            //test.Push(13);
            //test.Push(26);
            //t = test.Pop();
            //t = test.Pop();
        }

        class TestDetorClass
        {
            public TestDetorClass()
            {
                member = new A(72);
            }

            public A member;
        };

        static void testDestructorReference()
        {
            TestDetorClass a = new TestDetorClass();
        }

        static void testArraies()
        {
            A[] a = new A[14];
            a[0] = new A(22);
            a[0] = new A(21);
            a[13] = new A(14);
            a[13] = null;
        }

        static void testFinally()
        {
            Console.WriteLine("Testing finally clause");
            A a = new A(5);
            try
            {
                Console.WriteLine("In protected block");
                a.m_member = 6;
            }
            finally
            {
                Console.WriteLine("In finally!");
                a.m_member = 7;
            }
            Console.WriteLine("A should be 7: " + a.m_member);
        }

        static void testNestedFinally()
        {
            A a = new A(5);
            Console.WriteLine("Nested finally:");
            a.m_member = 0;
            try
            {
                a.m_member |= 1;
                try
                {
                    a.m_member |= 2;
                }
                finally
                {
                    a.m_member |= 4;
                }
                a.m_member |= 8;
            }
            finally
            {
                a.m_member |= 0x10;
                try
                {
                    a.m_member |= 0x20;
                }
                finally
                {
                    a.m_member |= 0x40;
                }
                a.m_member |= 0x80;
            }
            a.m_member |= 0x100;
            Console.WriteLine("A should be 511: " + a.m_member);
        }

        static void testCatchNoThrow()
        {
            Console.WriteLine("Testing catch clause");
            A a = new A(105);
            try
            {
                Console.WriteLine("In protected block");
                a.m_member = 106;
            }
            catch
            {
                Console.WriteLine("In catch!");
                a.m_member = 107;
            }
            Console.WriteLine("A should be 106: " + a.m_member);
            Console.WriteLine("Nested catch:");
            a.m_member = 0;
            try
            {
                a.m_member |= 1;
                try
                {
                    a.m_member |= 2;
                }
                catch
                {
                    a.m_member |= 4;
                }
                a.m_member |= 8;
            }
            finally
            {
                a.m_member |= 0x10;
                try
                {
                    a.m_member |= 0x20;
                }
                catch
                {
                    a.m_member |= 0x40;
                }
                a.m_member |= 0x80;
            }
            a.m_member |= 0x100;
            Console.WriteLine("A should be 443: " + a.m_member);
        }

        static void testCatchThrow()
        {
            A a;
            Console.WriteLine("Testing catch clause with throw");
            a = new A(0);
            try
            {
                Console.WriteLine("In protected block");
                throw new Exception("This is an exception");
                a.m_member |= 1;
            }
            catch
            {
                Console.WriteLine("In catch!");
                a.m_member |= 2;
            }
            Console.WriteLine("A should be 2: " + a.m_member);

            Console.WriteLine("Nested catch:");
            a = new A(0);
            try
            {
                Console.WriteLine("in try");
                a.m_member |= 1;
                try
                {
                    a.m_member |= 2;
                    throw new Exception("Ex1");
                }
                catch
                {
                    a.m_member |= 4;
                }
                a.m_member |= 8;
            }
            finally
            {
                Console.WriteLine("in finally");
                a.m_member |= 0x10;

                try
                {
                    Console.WriteLine("in finally: try");
                    a.m_member |= 0x20;
                    throw new Exception("Ex2");
                }
                catch
                {
                    Console.WriteLine("in finally: catch");
                    a.m_member |= 0x40;
                }
                a.m_member |= 0x80;
                Console.WriteLine("in finally: after");
            }
            a.m_member |= 0x100;
            Console.WriteLine("A should be 511: " + a.m_member);
        }

        static int bitMask = 0;
        static void throwerFunc(int bitValue, int nThrow)
        {
            // Junk for stack acivity
            A a = new A(555);
            A b = null;

            // Record this call in bitmask
            bitMask |= bitValue;

            // Allow (recursively) deep throw if positive value
            // Shallow throw if 1
            // No throw if 0 or negative
            if (nThrow > 1)
                throwerFunc(0, nThrow - 1);
            else if (nThrow == 1)
                throw new Exception("Yey");

            // More junk stack activity
            a.m_member += bitValue;
            b = new A(666);
        }

        static void testCatchDeep()
        {
            Console.WriteLine("Testing shallow throw-catch");
            bitMask = 0;
            throwerFunc(0x01, 0);
            try
            {
                throwerFunc(0x02, 0);
                throwerFunc(0x04, 1);
                throwerFunc(0x08, 0);
            }
            catch (Exception e)
            {
                Console.WriteLine("Caught: " + e.Message);
                throwerFunc(0x10, 0);
            }

            Console.WriteLine("Testing deep throw-catch");
            try
            {
                throwerFunc(0x02, 0);
                throwerFunc(0x04, 5);
                throwerFunc(0x08, 0);
            }
            catch (Exception e)
            {
                Console.WriteLine("Caught: " + e.Message);
                throwerFunc(0x10, 0);
            }
        }

        private class Faulter : IDisposable
        {
            public void Dispose()
            {
                Console.WriteLine("In Fault");
            }

            public static IEnumerator GetFaultEnumerator()
            {
                using (var f = new Faulter())
                {
                    Console.WriteLine("In Protected block");
                    yield return false;
                }
            }
        }

        static void testFault()
        {
            Faulter.GetFaultEnumerator().MoveNext();
        }


        static void testExceptions()
        {
            testFinally();
            testNestedFinally();
            testCatchNoThrow();
            testCatchThrow();
            testCatchDeep();
            // This test needs TABLE_TYPESPEC_TABLE support which is not perfect yet
            //testFault();
        }

        static void testLdobj()
        {
            Console.Write("testing ldobj / stobj...    ");

            int[] hashes = new int[5];
            hashes[1] |= 11;

            if (hashes[1] == 11)
            {
                Console.WriteLine("OK");
            }
            else
            {
                Console.WriteLine("FALSE");
            }
        }

        class MultipleReferenceObject
        {
            public int Value;

            public static int count = 0;

            MultipleReferenceObject[] arr;

            public MultipleReferenceObject()
            {
                Value = 15;
                count++;

                if (count > 100)
                    return;

                arr = new MultipleReferenceObject[10];
                for (int i = 0; i < 10; i++)
                    arr[i] = new MultipleReferenceObject();
            }

            ~MultipleReferenceObject()
            {
                count--;
                if (count == 0)
                    Console.WriteLine("Destructor works Great");
                if (count < 0)
                    Console.WriteLine("Oops, not so Great");
            }
        }

        static void testLinkConstrctors()
        {
            MultipleReferenceObject a = new MultipleReferenceObject();
            Console.WriteLine("Constructor ===> " + MultipleReferenceObject.count);
        }

        static void internalUnreferenceArguments(A outClass, A inClass)
        {
            outClass = inClass;
        }

        static void testUnreferenceArguments()
        {
            A arg1 = new A(6);
            A arg2 = new A(10);
            internalUnreferenceArguments(arg2, arg1);
            Console.WriteLine("Test score (should be 10): " + arg2);
        }

        public static int Compare(String strA, String strB, bool ignoreCase)
        {
            int i = 0;
            Byte A = (Byte)strA[i];
            Byte B = (Byte)strB[i];
            System.Console.Write("Before: A= " + A);    //prints 53 ('5')
            System.Console.WriteLine(", B=" + B);       //prints 53 ('5')
            if (ignoreCase)
            {
                if (A >= 'a' && A <= 'z')
                {
                    A = (Byte)(A - (Byte)32);          //This shouldn't Be executed!
                }

                if (B >= 'a' && B <= 'z')
                {
                    B = (Byte)(B - (Byte)32);          //This isn't executed (correct)
                }
            }

            System.Console.Write("After: A= " + A);
            System.Console.WriteLine(", B=" + B);


            if (strA.Length != strB.Length)             //interestingly: if this "if" is removed, then the code functions well
            {
                return (int)strA.Length - (int)strB.Length;
            }

            return 0;
        }

        static void testCompare()
        {
            string a = "5eb63bbbe01eeed09";
            string b = "5EB63BBBE01EEED09";

            int t = Compare(a, b, true);
        }

        ////////////////////////////////////////////
        [clrcore.Export("_myMain"), clrcore.CallingConvention("cdecl")]
        static unsafe void Main()
        {
            testCompare();
            testUnreferenceArguments();
            testLdobj();
            testDestructorReference();
            testLocalsOverwrite1();
            testClassed();
            testLocalsCleanup();
            testFieldRVA();
            testHashCode();
            testBoxing();
            testInterfaces();
            testInterfaces1();
            testLongStrings();
            testArraies();
            testLinkConstrctors();
            testExceptions();
        }
    }
}

namespace Morph
{
    class Console
    {
        public static unsafe void Write(Morph.String str)
        {
            if (str == null)
                return;

            for (uint i = 0; i < str.m_size; i++)
            {
                Morph.Imports.debugChar((char)(str.m_buffer[i]));
            }
        }

        public static unsafe void Write(Object obj)
        {
            if (obj == null)
                return;
            string str = obj.ToString();

            System.Console.Write(str);
        }

        public static unsafe void Write(bool i)
        {
            System.Console.Write(i.ToString());
        }

        public static unsafe void Write(char ch)
        {
            Morph.Imports.debugChar(ch);
        }

        public static unsafe void Write(int i)
        {
            System.Console.Write(i.ToString());
        }

        public static unsafe void Write(uint i)
        {
            System.Console.Write(i.ToString());
        }

        public static unsafe void Write(ushort i)
        {
            System.Console.Write(i.ToString());
        }

        public static unsafe void Write(short i)
        {
            System.Console.Write(i.ToString());
        }

        public static unsafe void Write(byte i)
        {
            System.Console.Write(i.ToString());
        }

        public static unsafe void Write(sbyte i)
        {
            System.Console.Write(i.ToString());
        }


        public static unsafe void WriteLine()
        {
            Morph.Imports.debugChar('\r');
            Morph.Imports.debugChar('\n');
        }

        public static unsafe void WriteLine(Morph.String str)
        {
            Write(str);
            WriteLine();
        }

        public static unsafe void WriteLine(Object obj)
        {
            if (obj == null)
                return;
            string str = obj.ToString();

            System.Console.WriteLine(str);
        }

        public static unsafe void WriteLine(bool i)
        {
            System.Console.WriteLine(i.ToString());
        }

        public static unsafe void WriteLine(int number)
        {
            System.Console.WriteLine(number.ToString());
        }

        public static unsafe void WriteLine(char ch)
        {
            Write(ch);
            WriteLine();
        }

        public static unsafe void WriteLine(uint number)
        {
            System.Console.WriteLine(number.ToString());
        }

        public static unsafe void WriteLine(short number)
        {
            System.Console.WriteLine(number.ToString());
        }

        public static unsafe void WriteLine(ushort number)
        {
            System.Console.WriteLine(number.ToString());
        }

        public static unsafe void WriteLine(byte number)
        {
            System.Console.WriteLine(number.ToString());
        }

        public static unsafe void WriteLine(sbyte number)
        {
            System.Console.WriteLine(number.ToString());
        }
    }
}

namespace TestCustomAttributes
{
    [System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = false)]
    public class CallingConvention : System.Attribute
    {
        private string name;
        private int moshe;
        private bool x;

        public CallingConvention(string name, int moshe = 5, bool x = true)
        {
            this.name = name;
            this.moshe = moshe;
            this.x = x;
        }
    }

    [System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
    public class Debug : System.Attribute
    {
        private string facility;
        private bool color;

        public Debug(string facility, bool color = true)
        {
            this.facility = facility;
            this.color = color;
        }
    }

    class Program
    {
        [CallingConvention("cdecl", 2)]
        static int moshe()
        {
            return 5;
        }

        [CallingConvention("stdcall"), Debug("haim!", true), Debug("DingDong")]
        static int haim()
        {
            return 5;
        }

        static void Main()
        {
            moshe();
            haim();
        }
    }
}

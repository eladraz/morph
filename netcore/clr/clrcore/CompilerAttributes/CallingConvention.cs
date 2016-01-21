namespace clrcore
{
    [System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = false)]
    public class CallingConvention : System.Attribute
    {
        private string callingConvention;

        public CallingConvention(string callingConvention)
        {
            this.callingConvention = callingConvention;
        }
    }

}

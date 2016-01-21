namespace Morph
{
    public class Exception
    {
        internal int hresult = -2146233088;

        //System.Collections.IDictionary data;
        public string HelpLink { get; set; }
        public Exception InnerException { get; set; }
        public virtual string Message { get; set; }
        public string Source { get; set; }
        public string StackTrace { get; set; }
        //System.Reflection.MethodBase TargetSite;

        public Exception(): this(null)
        {
        }
        public Exception(string message) : this(message, null)
        {
            Message = message;
        }
        public Exception(string message, Exception innerException)
        {
            InnerException = innerException;
            Message = message;
            if (message != null)
            {
                System.Console.WriteLine(message);
            }
        }
        public Exception GetBaseException()
        {
            return this;
        }

        public override string ToString()
        {
            return "Exception: " + Message;
        }


        //public void GetObjectData(System.Runtime.Serialization.SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
        //{
        //}

        protected int HResult
        {
            get { return hresult; }
            set { hresult = value; }
        }

    }
}

namespace Morph
{
    public class ArgumentOutOfRangeException : ArgumentException
    {
        const int Result = unchecked((int)0x80131502);

        private object actual_value;

        // Constructors
        public ArgumentOutOfRangeException()
            : base("Argument is out of range.")
        {
            HResult = Result;
        }

        public ArgumentOutOfRangeException(string paramName)
            : base("Argument is out of range.", paramName)
        {
            HResult = Result;
        }

        public ArgumentOutOfRangeException(string paramName, string message)
            : base(message, paramName)
        {
            HResult = Result;
        }

        public ArgumentOutOfRangeException(string paramName, object actualValue, string message)
            : base(message, paramName)
        {
            this.actual_value = actualValue;
            HResult = Result;
        }

        public ArgumentOutOfRangeException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }

        // Properties
        public virtual object ActualValue
        {
            get
            {
                return actual_value;
            }
        }

        public override string Message
        {
            get
            {
                string basemsg = base.Message;
                if (actual_value == null)
                    return basemsg;
                return basemsg + "\r\n" + actual_value;
            }
        }
    }
}

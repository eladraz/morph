namespace Morph
{
    public class ArgumentException : SystemException
    {
        const int Result = unchecked((int)0x80070057);

        private string param_name;

        // Constructors
        public ArgumentException()
            : base("Value does not fall within the expected range.")
        {
            HResult = Result;
        }

        public ArgumentException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public ArgumentException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }

        public ArgumentException(string message, string paramName)
            : base(message)
        {
            this.param_name = paramName;
            HResult = Result;
        }

        public ArgumentException(string message, string paramName, Exception innerException)
            : base(message, innerException)
        {
            this.param_name = paramName;
            HResult = Result;
        }

        // Properties
        public virtual string ParamName
        {
            get
            {
                return param_name;
            }
        }

        public override string Message
        {
            get
            {
                if (ParamName != null && ParamName.Length != 0)
                    return base.Message + "\r\n"
                        + "Parameter name: "
                        + ParamName;
                return base.Message;
            }
        }
    }
}

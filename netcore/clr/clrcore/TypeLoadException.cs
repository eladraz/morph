namespace Morph
{
    public class TypeLoadException : SystemException
    {
        const int Result = unchecked((int)0x80131522);

        // Fields
        private string className, assemblyName;

        // Constructors
        public TypeLoadException()
            : base("A type load exception has occurred.")
        {
            HResult = Result;
        }

        public TypeLoadException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public TypeLoadException(string message, Exception inner)
            : base(message, inner)
        {
            HResult = Result;
        }

        internal TypeLoadException(string className, string assemblyName)
            : this()
        {
            this.className = className;
            this.assemblyName = assemblyName;
        }

        // Properties
        public override string Message
        {
            get
            {
                if (className != null)
                {
                    if ((assemblyName != null) && (assemblyName != String.Empty))
                        return "Could not load type '" + className + "' from assembly '" + assemblyName + "'.";
                    else
                        return "Could not load type '" + className + "'.";
                }
                else
                    return base.Message;
            }
        }

        public string TypeName
        {
            get
            {
                if (className == null)
                    return String.Empty;
                else
                    return className;
            }
        }
    }
}

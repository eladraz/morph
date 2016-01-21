namespace Morph
{
    public class Object
    {
        public Object()
        {
        }

        public virtual void Finalize()
        {
        }

        public virtual bool Equals(object obj)
        {
            return false;
        }

        public virtual int GetHashCode()
        {
            return 0;
        }

        public virtual string ToString()
        {
            return "Morph.Object";
        }

        public static implicit operator string(Object value)
        {
            return Imports.convertToString(value.ToString());
        }

        protected object MemberwiseClone()
        {
            return null; // throw new System.NotImplementedException();
        }


        /*
         *  TODO!
         *
        public static bool ReferenceEquals(object obj1,
                                           object obj2)
        {
            return false;
        }
        public static bool Equals(object obj1,
                                   object obj2);

        public Type GetType();
         */
    }
}

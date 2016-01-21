namespace Morph
{
    public class ValueType
    {
        public ValueType()
        {
        }

        /*
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
            return null;
        }*/

        [clrcore.CompilerOpcodes("getRTTI")]
        public static ushort getRTTI()
        {
            return 0;
        }
    }
}

namespace Morph.Runtime.CompilerServices
{
    class RuntimeHelpers
    {
        [clrcore.CompilerOpcodes("storeVar")]
        static void InitializeArray(Morph.Array array, Morph.RuntimeFieldHandle fieldHandle)
        {
           // Implemented inside the compiler as memcpy, via a call to storeVar
        }
    }
}

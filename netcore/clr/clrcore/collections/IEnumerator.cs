

namespace Morph.Collections
{
    public interface IEnumerator
    {
        System.Object Current { get; }
        bool MoveNext();
        void Reset();
    }
}

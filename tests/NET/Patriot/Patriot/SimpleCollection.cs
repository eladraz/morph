using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace Patriot
{
    class SimpleCollection : ICollection
    {
        private object[] _items = null;

        class SimpleCollectionEnumerator : IEnumerator
        {
            int m_index;
            object[] _items;
            public SimpleCollectionEnumerator(object[] items)
            {
                _items = items;
                m_index = -1;
            }
            public object Current
            {
                get {
                    return _items[m_index];
                }
            }

            public bool MoveNext()
            {
                m_index++;

                while (true)
                {
                    if (_items[m_index] != null)
                    {
                        return true;
                    }
                    m_index++;
                    if (m_index >= _items.Length)
                    {
                        return false;
                    }
                }
            }

            public void Reset()
            {
                throw new NotImplementedException();
            }
        }


        public SimpleCollection(int capacity)
        {
            _items = new object[capacity];
        }


        public void CopyTo(Array array, int index)
        {
            throw new NotImplementedException();
        }

        public int Count
        {
            get
            {
                int count = 0;
                for (int i = 0; i < _items.Length; i++)
                {
                    count++;
                }
                return count;
            }
        }

        public bool IsSynchronized
        {
            get { throw new NotImplementedException(); }
        }

        public object SyncRoot
        {
            get { throw new NotImplementedException(); }
        }

        public IEnumerator GetEnumerator()
        {
            return new SimpleCollectionEnumerator(_items);
        }

        public void Add(object obj)
        {
            _items[getNextFreeIndex()] = obj;
        }

        private int getNextFreeIndex()
        {
            for (int i = 0; i < _items.Length; i++)
            {
                if (_items[i] == null)
                {
                    return i;
                }
            }
            throw new Exception("Simple Collection out of capacity");
        }

        public void Remove(object obj)
        {
            for (int i = 0; i < _items.Length; i++)
            {
                if (_items[i] == obj)
                {
                    _items[i] = null;
                }
            }
        }

    }
}

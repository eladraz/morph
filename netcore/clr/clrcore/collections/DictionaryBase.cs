namespace Morph.Collections
{
    public abstract class DictionaryBase : System.Collections.IDictionary, System.Collections.ICollection, System.Collections.IEnumerable
    {

        System.Collections.Hashtable hashtable;

        protected DictionaryBase()
        {
            hashtable = new System.Collections.Hashtable();
        }

        public void Clear()
        {
            OnClear();
            hashtable.Clear();
            OnClearComplete();
        }

        public int Count
        {
            get
            {
                return hashtable.Count;
            }
        }

        public bool IsSynchronized
        {
            get
            {
                return hashtable.IsSynchronized;
            }
        }

        protected System.Collections.IDictionary Dictionary
        {
            get
            {
                return this;
            }
        }

        protected System.Collections.Hashtable InnerHashtable
        {
            get
            {
                return hashtable;
            }
        }

        public void CopyTo(System.Array array, int index)
        {
            if (array == null)
                throw new System.ArgumentNullException("array");
            if (index < 0)
                throw new System.ArgumentOutOfRangeException("index must be possitive");
            if (array.Rank > 1)
                throw new System.ArgumentException("array is multidimensional");
            int size = array.Length;
            if (index > size)
                throw new System.ArgumentException("index is larger than array size");
            if (index + Count > size)
                throw new System.ArgumentException("Copy will overlflow array");

            DoCopy(array, index);
        }

        private void DoCopy(System.Array array, int index)
        {
            foreach (DictionaryEntry de in hashtable)
                array.SetValue(de, index++);
        }

        public System.Collections.IDictionaryEnumerator GetEnumerator()
        {
            return hashtable.GetEnumerator();
        }

        protected virtual void OnClear()
        {
        }

        protected virtual void OnClearComplete()
        {
        }

        protected virtual object OnGet(object key, object currentValue)
        {
            return currentValue;
        }

        protected virtual void OnInsert(object key, object value)
        {
        }

        protected virtual void OnInsertComplete(object key, object value)
        {
        }

        protected virtual void OnSet(object key, object oldValue, object newValue)
        {
        }

        protected virtual void OnSetComplete(object key, object oldValue, object newValue)
        {
        }

        protected virtual void OnRemove(object key, object value)
        {
        }

        protected virtual void OnRemoveComplete(object key, object value)
        {
        }

        protected virtual void OnValidate(object key, object value)
        {
        }

        bool System.Collections.IDictionary.IsFixedSize
        {
            get
            {
                return false;
            }
        }

        bool System.Collections.IDictionary.IsReadOnly
        {
            get
            {
                return false;
            }
        }

        object System.Collections.IDictionary.this[object key]
        {
            get
            {
                object value = hashtable[key];
                OnGet(key, value);
                return value;
            }
            set
            {
                OnValidate(key, value);
                object current_value = hashtable[key];
                OnSet(key, current_value, value);
                hashtable[key] = value;
                try
                {
                    OnSetComplete(key, current_value, value);
                }
                catch
                {
                    hashtable[key] = current_value;
                    throw;
                }
            }
        }

        System.Collections.ICollection System.Collections.IDictionary.Keys
        {
            get
            {
                return hashtable.Keys;
            }
        }

        System.Collections.ICollection System.Collections.IDictionary.Values
        {
            get
            {
                return hashtable.Values;
            }
        }

        void System.Collections.IDictionary.Add(object key, object value)
        {
            OnValidate(key, value);
            OnInsert(key, value);
            hashtable.Add(key, value);
            try
            {
                OnInsertComplete(key, value);
            }
            catch
            {
                hashtable.Remove(key);
                throw;
            }
        }

        void System.Collections.IDictionary.Remove(object key)
        {
            if (!hashtable.Contains(key))
                return;

            object value = hashtable[key];
            OnValidate(key, value);
            OnRemove(key, value);
            hashtable.Remove(key);
            try
            {
                OnRemoveComplete(key, value);
            }
            catch
            {
                hashtable[key] = value;
                throw;
            }
        }

        bool System.Collections.IDictionary.Contains(object key)
        {
            return hashtable.Contains(key);
        }

        //bool ICollection.IsSynchronized
        //{
        //    get
        //    {
        //        return hashtable.IsSynchronized;
        //    }
        //}

        object System.Collections.ICollection.SyncRoot
        {
            get
            {
                return hashtable.SyncRoot;
            }
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return hashtable.GetEnumerator();
        }
    }
}

namespace TestArray
{
    class TestArray
    {
        static int test_allocate()
        {
            int[] arr;
            arr = new int[1024];
            arr = new int[2048];
            arr = new int[4096];
            arr = new int[8192];
            return 0;
        }

        static int test_store_load_match_const()
        {
            int[] arr;
            arr = new int[10];

            for (int i = 0; i < 10; i++)
            {
                arr[i] = i;
            }

            for (int i = 0; i < 10; i++)
            {
                if (i != arr[i])
                {
                    return -1;
                }
            }

            return 0;
        }

        static int test_store_load_match_element()
        {
            int[] arr, arr2;
            arr = new int[10];
            arr2 = new int[10];

            for (int i = 0; i < 10; i++)
            {
                arr[i] = i;
                arr2[i] = i;
            }

            for (int i = 0; i < 10; i++)
            {
                if (arr[i] != arr2[i])
                {
                    return -1;
                }
            }

            return 0;
        }

        static int test_hashcode()
        {
            int[] arr;
            arr = new int[10];

            for (int i = 0; i < 10; i++)
            {
                arr[i] = i;
            }

            for (int i = 0; i < 10; i++)
            {
                // FIX ASAP: BUG!
                //if (i != arr[i].GetHashCode() || (i != arr[i]))
                //{
                //    return -1;
                //}

                int hashcode = arr[i].GetHashCode();
                if (i !=  hashcode || (i != arr[i]))
                {
                    return -1;
                }
            }

            return 0;
        }

        static int test_dynamic_size()
        {
            int i = 5;
            int[] arr;
            arr = new int[i];
            return 0;
        }

        static int Main()
        {
            bool failed = false;

            TBA.Debug.debugString("TestArray\n");
            TBA.Debug.debugString("=============\n");
            TBA.Debug.debugString("\n");

            if (0 != test_allocate())
            {
                TBA.Debug.debugString("test_allocate: not ok.\n");
                failed = true;
            }
            TBA.Debug.debugString("test_allocate: ok.\n");

            if (0 != test_dynamic_size())
            {
                TBA.Debug.debugString("test_dynamic_size: not ok.\n");
                failed = true;
            }
            TBA.Debug.debugString("test_dynamic_size: ok.\n");

            if (0 != test_store_load_match_const())
            {
                TBA.Debug.debugString("test_store_load_match_const: not ok.\n");
                failed = true;
            }
            TBA.Debug.debugString("test_store_load_match_const: ok.\n");

            if (0 != test_store_load_match_element())
            {
                TBA.Debug.debugString("test_store_load_match_element: not ok.\n");
                failed = true;
            }
            TBA.Debug.debugString("test_store_load_match_element: ok.\n");

            if (0 != test_hashcode())
            {
                TBA.Debug.debugString("test_hashcode: not ok.\n");
                failed = true;
            }
            TBA.Debug.debugString("test_hashcode: ok.\n");

            if (failed)
            {
                return -1;
            }

            TBA.Debug.debugString("\n");
            TBA.Debug.debugString("ALL OK!\n");
            return 0;
        }
    }
}

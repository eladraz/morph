namespace clrcore
{
    class ExceptionHandling
    {
        /* Can't use enum yet
        private enum EntryType
        {
            MethodCleanup,
            FinallyHandler,
            FaultHandler,
            Catch,
            Filter,
        }*/
        private const int MethodCleanup = 1;
        private const int FinallyHandler = 2;
        private const int FaultHandler = 3;
        private const int Catch = 4;
        private const int Filter = 5;

        private unsafe struct Entry
        {
            // The type of entry - see constants above
            public uint type;
            // Handler routine function pointer
            public void* handler;
            // The exception class RTTI number, if this is a catch handler
            public ushort exceptionRtti;
            // The filter routine function pointer, if this is a filter handler
            public void* filter;
            // Stack pointer to use when running the handler or filter routine
            public void* stackPointer;
            // Stack pointer to use when returning from a catch or filter handler
            public void* stackPointerRet;
            // Pointer to the next entry in the linked-list
            public Entry* next;
        }

        // Pointer to the head of the exception stack
        private unsafe static Entry* exceptionStack = null;

        // Current thrown exception
        private unsafe static Morph.Exception currentException = null;
        // stack frame pointer of the catch context
        private unsafe static void* currentStackPointerRet = null;

        private unsafe static void registerRoutine(uint type, void* handler, void* stackPointer)
        {
            // Allocate a new entry for the linked list
            Entry* entry = (Entry*)Morph.Imports.allocate((uint)sizeof(Entry));

            // Fill in the entry according to arguments
            entry->type = type;
            entry->handler = handler;
            entry->filter = null;
            entry->exceptionRtti = 0;
            entry->stackPointer = stackPointer;
            entry->stackPointerRet = null;

            // Link the new entry as the new head of the stack
            entry->next = exceptionStack;
            exceptionStack = entry;
        }

        private unsafe static void registerCatch(ushort exceptionRtti, void* handler, void* stackPointer, void* stackPointerRet)
        {
            // Allocate a new entry for the linked list
            Entry* entry = (Entry*)Morph.Imports.allocate((uint)sizeof(Entry));

            // Fill in the entry according to arguments
            entry->type = Catch;
            entry->handler = handler;
            entry->filter = null;
            entry->exceptionRtti = exceptionRtti;
            entry->stackPointer = stackPointer;
            entry->stackPointerRet = stackPointerRet;

            // Link the new entry as the new head of the stack
            entry->next = exceptionStack;
            exceptionStack = entry;
        }

        private unsafe static void registerFilter(void* filter, void* handler, void* stackPointer, void* stackPointerRet)
        {
            // Allocate a new entry for the linked list
            Entry* entry = (Entry*)Morph.Imports.allocate((uint)sizeof(Entry));

            // Fill in the entry according to arguments
            entry->type = Filter;
            entry->handler = handler;
            entry->filter = filter;
            entry->exceptionRtti = 0;
            entry->stackPointer = stackPointer;
            entry->stackPointerRet = stackPointerRet;

            // Link the new entry as the new head of the stack
            entry->next = exceptionStack;
            exceptionStack = entry;
        }

        private unsafe static void popAndExecute()
        {
            // Fetch the stack's head
            System.Diagnostics.Debug.Assert(exceptionStack != null);
            if (exceptionStack == null)
                return;

            void* handler = exceptionStack->handler;
            void* stackPointer = exceptionStack->stackPointer;

            // Pop it from the stack and deallocate it
            pop();

            // Execute the handler
            Morph.Imports.callFunction(handler, stackPointer);
        }

        private unsafe static void pop()
        {
            // Fetch the stack's head
            Entry* entry = exceptionStack;
            System.Diagnostics.Debug.Assert(entry != null);
            if (entry == null)
                return;

            // Unlink from the stack
            exceptionStack = entry->next;

            // Deallocate
            Morph.Imports.free((void*)entry);
        }

        private unsafe static void* getStackPointerRet()
        {
            return currentStackPointerRet;
        }

        private static Morph.Exception getCurrentException()
        {
            return currentException;
        }

        private unsafe static void throwException(Morph.Exception exceptionObject)
        {
            // Switch from NULL to NullReferenceException
            if (exceptionObject == null)
                exceptionObject = new Morph.NullReferenceException();

            currentException = exceptionObject;
            void* exceptionVtbl = clrcore.GarbageCollector.garbageCollectorGetVTbl(Morph.Imports.convert(exceptionObject));
            while (exceptionStack != null)
            {
                bool bExecuteHandler = false;
                if (exceptionStack->type == Filter)
                {
                    // Filter - run filter code. decide to catch or not
                    // bExecuteHandler = CallFilter()
                    Morph.Diagnostics.Debug.Assert(false);
                }
                else if (exceptionStack->type == Catch)
                {
                    if (VirtualTable.virtualTableGetInterfaceLocation(exceptionVtbl, exceptionStack->exceptionRtti) != 0xFFFFFFFF)
                    {
                        // Exception caught!
                        bExecuteHandler = true;
                        currentStackPointerRet = exceptionStack->stackPointerRet;
                    }
                }
                else if ((exceptionStack->type == FinallyHandler) || (exceptionStack->type == FaultHandler) || (exceptionStack->type == MethodCleanup))
                {
                    // Execute all method cleanups, finallies, and faults
                    bExecuteHandler = true;
                }

                // run handler. Note: if this is a catch handler, it will not return.
                // instead it will jump!
                if (bExecuteHandler)
                    popAndExecute();
                else
                    pop();
            }
            // Unhandled exception!
            System.Console.WriteLine("Unhandled exception:");
            System.Console.WriteLine(exceptionObject.Message);
            Morph.Imports.unhandledException(exceptionObject);
        }
    }
}

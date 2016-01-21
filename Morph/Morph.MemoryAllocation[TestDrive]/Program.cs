using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Morph.MemoryAllocation.TestDrive
{
    using Size = UInt32;

    unsafe class Program
    {
        struct pool
        {
            internal fixed int area[1024];

            internal unsafe RAM AsRAM()
            {
                //CR: why not make this line with a parameter instead? wierd use of &this.
                fixed (pool* p = &this) {
                    var ram = new RAM { start = p->area, sz = (uint)sizeof(pool) };
                    Console.WriteLine("RAM @ 0x{0:x}", new IntPtr(ram.start).ToInt32());
                    Console.WriteLine("   sz {0} bytes", ram.sz);
                    return ram;
                }
            }
        }

        pool p = new pool();


        unsafe struct Link
        {
            public Int32 value;
            public Link *next;
        }

        unsafe void TestFreeBits()
        {
            fixed (pool* p = &this.p) {
                FreeBits fb = new FreeBits();

                fb.Initialize(p->AsRAM(), 16);

                var a = fb.Alloc(64);
                Console.WriteLine("a @ 0x{0:x}", (int)a);
                var ai =  ((int*)a);

                var b = fb.Alloc(96);
                Console.WriteLine("b @ 0x{0:x}", (int)b);
                var bi = ((int*)b);

                /* Fill test area */
                for (int i = 0; i < 16; ++i)
                    ai[i] = (i + 1) * (i + 1);
                for (int i = 0; i < 24; ++i)
                    bi[i] = (i + 1) * (i + 2);
                /* Print test area */
                for (int i = 0; i < 16; ++i)
                    Console.WriteLine("a[{0}] = {1}", i, ai[i]);

                for (int i = 0; i < 24; ++i)
                    Console.WriteLine("b[{0}] = {1}", i, bi[i]);


                /* Create a linked list */
                Link* head = null;
                for (int i = 0; i < 16; ++i) {
                    Link* link = (Link*)(fb.Alloc((Size)sizeof(Link)));
                    link->value = i + 8;
                    link->next = head;
                    head = link;
                }

                /* Read linked list */
                for (Link* link = head; link != null; link = link->next) {
                    Console.Write(" {0} ->", link->value);
                }
                Console.WriteLine(" |>");
            }
        }

        unsafe void TestFreeList()
        {
            fixed (pool* p = &this.p) {
                FreeList fl = new FreeList();

                fl.Initialize(p->AsRAM(), 16);

                var a = fl.Alloc(16);
                var b = fl.Alloc(16);
                var c = fl.Alloc(16);

                fl.Free(b);

                var d = fl.Alloc(16);
                var e = fl.Alloc(16);

                Console.WriteLine("a @ 0x{0:x}", (int)a);
                Console.WriteLine("b @ 0x{0:x}", (int)b);
                Console.WriteLine("c @ 0x{0:x}", (int)c);
                Console.WriteLine("d @ 0x{0:x}", (int)d);
                Console.WriteLine("e @ 0x{0:x}", (int)e);

                fl.Free(d);
                fl.Free(c);

                var f = fl.Alloc(16);

                Console.WriteLine("f @ 0x{0:x}", (int)f);
                Console.WriteLine("/Memory Map/");
                Console.WriteLine(fl.MemoryMap);
            }
        }

        unsafe void TestBuddyTechnique()
        {
            fixed (pool* p = &this.p) {
                var bt = new BuddyTechnique();

                bt.Initialize(p->AsRAM(), 512, 16);

                var a = bt.Alloc(32);
                var b = bt.Alloc(32);

                var c = bt.Alloc(64);
                var d = bt.Alloc(64);

                Console.WriteLine("a @ 0x{0:x}", (int)a);
                Console.WriteLine("b @ 0x{0:x}", (int)b);

                bt.Free(a);
                bt.Free(b);
                bt.Free(d);
                bt.Free(c);

                Console.WriteLine("/Memory Map/");
                Console.WriteLine(bt.MemoryMap);
                Console.WriteLine();
                foreach (var s in bt.SubMemoryMaps)
                    Console.WriteLine(s);
            }
        }


        void Go()
        {
            //TestFreeBits();
            //TestFreeList();
            TestBuddyTechnique();
        }

        static void Main(string[] args)
        {
            new Program().Go();
        }
    }
}

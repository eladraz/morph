using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [clrcore.Export("_myMain"), clrcore.CallingConvention("cdecl")]
        static void Main()
        {
            try
            {
                IGameControl control = new MorphControl();
                IGraphicsAdapter display = new MorphDisplay();

                PatriotGame patriot = new PatriotGame();
                patriot.Init(display, control);

                while (true)
                {
                    patriot.Tick();
                    Delay();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("exception encountered");
                Console.WriteLine(ex.Message);
            }
        }

        static void Delay()
        {
             for (uint i = 200000; i > 0; i--) ;
        }
    }
}

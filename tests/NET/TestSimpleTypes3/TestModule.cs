using System;

namespace TestSimpleTypes3
{
	/// <summary>
	/// Execute tests
	/// </summary>
	class Test
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
            testHumman();
            int j = 0;
            j++;
		}

        static void printCreator(Creator creator)
        {
            System.Console.WriteLine("Type: " + creator.GetType().ToString());
            System.Console.WriteLine("Name: " + creator.getName());
            System.Console.WriteLine("Age:  " + creator.getAge().ToString());
            creator.eat();
        }

        static void printPosition(ObjectPosition obj)
        {
            System.Console.WriteLine("Position:  " + obj.getPosition().ToString());
        }

        static void testHumman()
        {
            Humman razi = new Humman("Razi", 23);
            System.Console.WriteLine("RAZI");
            System.Console.WriteLine("====");
            printCreator(razi);
            Humman person = razi;
            printCreator(person);
            printCreator(razi);
            printPosition(razi);
            for (uint i = 0; i < 120; i++)
                razi.eat();
        }
	}
}

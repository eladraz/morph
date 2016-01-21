//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using Morph;

//namespace Patriot
//{
//    class StellarisImports
//    {
//        //Game Controls
//        [clrcore.Import("stellaris_get_potentiometer")]
//        public static int GetPotentiometer()
//        {
//            return 1;
//        }

//        [clrcore.Import("stellaris_get_btn_pressed")]
//        public static bool GetButtonPressed()
//        {
//            return true;
//        }

//        [clrcore.Import("stellaris_is_keypressed")]
//        public static void IsKeyPressed(int keyCode) { }

//        //Display
//        [clrcore.Import("stellaris_display_get_height")]
//        public static int GetHeight()
//        {
//            return 1;
//        }

//        [clrcore.Import("stellaris_display_get_height")]
//        public static int GetWidth()
//        {
//            return 1;
//        }

//        [clrcore.Import("stellaris_display_draw_rectangle")]
//        public static void DrawRectangle(int x, int y, int width, int height, int color)
//        {

//        }

//        [clrcore.Import("stellaris_display_fill_rectangle")]
//        public static void FillRectangle(int x, int y, int width, int height, int color)
//        {
//            int a;
//            int b = 5;
//            int c = 100;
//            a = c  >  ( b < 10 ? 1 : 15) ? 1 : 10;

//            a =  b > 3 ? b < 5 ? 4 : -1  : 10 ;
//        }


//    }
//}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    public static class GameConsts
    {
        public static int stepCount;

        public static int Gravity;

        public static int MissileSpeedX = -3;
        public static int MissileSpeedY = -2;

        public static int maxPatriotsInAir = 15;
        public static int maxPatriotBurst = 1;

        public static int PatriotBaseSpeed = 20;

        public static int PatriotLength = 15;
        public static int PatriotWidth = 7;

        public static int TurretRadius = 50;
        public static int TurretTurenStep = 5;
        public static int KillRadius = 15;
        public static int GravityFloatAdaptaion = 3;
    }
}

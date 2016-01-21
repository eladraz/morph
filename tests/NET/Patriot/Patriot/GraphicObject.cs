using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    class GraphicObject
    {
        protected IGraphicsAdapter m_graphics;

        public GraphicObject(IGraphicsAdapter _grapihcs)
        {
            m_graphics = _grapihcs;
        }

        internal virtual void Draw() { }
        internal virtual void UnDraw() { }
    }
}

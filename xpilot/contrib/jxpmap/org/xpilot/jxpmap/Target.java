/*
 * $Id$
 */
package org.xpilot.jxpmap;

import java.awt.Polygon;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;

public class Target extends MapPolygon {
    
    public Target() {
        super();
    }

    public Target(Polygon p, PolygonStyle style, ArrayList edgeStyles, 
    int team) {
        super(p, style, edgeStyles);
        setTeam(team);
    }

    public void printXml(PrintWriter out) throws IOException {
        out.println("<Target team=\"" + getTeam() + "\">");
        super.printXml(out);
        out.println("</Target>");
    }

    public EditorPanel getPropertyEditor(MapCanvas canvas) {
        CompoundEditor ce = 
            new CompoundEditor("BallArea", canvas, this);
        ce.add(new TeamEditor("Team", canvas, this));
        ce.add(super.getPropertyEditor(canvas));
        return ce;
    }    
}

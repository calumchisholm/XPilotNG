/*
 * $Id$
 */
package org.xpilot.jxpmap;

import java.awt.Polygon;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;

/**
 * @author jli
 */
public class BallTarget extends MapPolygon {
    
    public BallTarget() {
        super();
    }
    
    public BallTarget(Polygon p, PolygonStyle style, ArrayList edgeStyles,
    int team) {
        super(p, style, edgeStyles);
        setTeam(team);
    }

    public void printXml(PrintWriter out) throws IOException {
        out.println("<BallTarget team=\"" + getTeam() + "\">");
        super.printXml(out);
        out.println("</BallTarget>");
    }

    public EditorPanel getPropertyEditor(MapCanvas canvas) {
        CompoundEditor ce = 
            new CompoundEditor("BallTarget", canvas, this);
        ce.add(new TeamEditor("Team", canvas, this));
        ce.add(super.getPropertyEditor(canvas));
        return ce;
    }
}

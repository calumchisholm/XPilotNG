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
public class BallArea extends MapPolygon {
    
    public BallArea() {
        super();
    }   
    
    public BallArea(Polygon p, PolygonStyle style, ArrayList edgeStyles) {
        super(p, style, edgeStyles);
    }

    public void printXml(PrintWriter out) throws IOException {
        out.println("<BallArea>");
        super.printXml(out);
        out.println("</BallArea>");
    }
}

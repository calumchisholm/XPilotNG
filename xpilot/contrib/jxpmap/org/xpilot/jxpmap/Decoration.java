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
public class Decoration extends MapPolygon  {
    
    public Decoration() {
        super();
    }

    public Decoration(Polygon p, PolygonStyle style, ArrayList edgeStyles) {
        super(p, style, edgeStyles);
    }

    public void printXml(PrintWriter out) throws IOException {
        out.println("<Decor>");
        super.printXml(out);
        out.println("</Decor>");
    }
}

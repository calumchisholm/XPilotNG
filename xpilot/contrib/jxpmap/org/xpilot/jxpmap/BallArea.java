/*
 * $Id$
 */
package org.xpilot.jxpmap;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Collection;

/**
 * @author jli
 */
public class BallArea extends Group {
    
    public BallArea() {
        super();
    }   
    
    public BallArea(Collection c) {
        super(c);
    }

    public void printXml(PrintWriter out) throws IOException {
        out.println("<BallArea>");
        super.printMemberXml(out);
        out.println("</BallArea>");
    }
}

package org.xpilot.jxpmap;

import java.io.IOException;
import java.io.PrintWriter;

public class CheckPoint extends MapObject {


    public CheckPoint () {
        this(0, 0);
    }


    public CheckPoint (int x, int y) {
        super("checkpoint.gif", x, y, 35 * 64, 35 * 64);
    }


    public void printXml (PrintWriter out) throws IOException {
        out.print("<Check x=\"");
        out.print(getBounds().x + getBounds().width / 2);
        out.print("\" y=\"");
        out.print(getBounds().y + getBounds().height / 2);
        out.println("\"/>");
    }
}

package org.xpilot.jxpmap;

import java.io.PrintWriter;
import java.io.IOException;

public class MapCheckPoint extends MapObject {


    public MapCheckPoint () {
        this(0, 0);
    }


    public MapCheckPoint (int x, int y) {
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

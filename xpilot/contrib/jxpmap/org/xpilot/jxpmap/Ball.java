package org.xpilot.jxpmap;

import java.io.IOException;
import java.io.PrintWriter;

public class Ball extends MapObject {

    public Ball () {
        this(0, 0, 1);
    }

    
    public Ball (int x, int y, int team) {
        super("ball.gif", x, y, 21 * 64, 21 * 64);
        setTeam(team);
    }


    public void printXml (PrintWriter out) throws IOException {
        out.print("<Ball x=\"");
        out.print(getBounds().x + getBounds().width / 2);
        out.print("\" y=\"");
        out.print(getBounds().y + getBounds().height / 2);
        out.print("\" team=\"");
        out.print(getTeam());
        out.println("\"/>");
    }


    public EditorPanel getPropertyEditor (MapCanvas canvas) {
        return new TeamEditor("Ball", canvas, this);
    }
}

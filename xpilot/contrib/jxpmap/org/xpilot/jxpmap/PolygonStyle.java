package org.xpilot.jxpmap;

import java.awt.Color;
import java.io.IOException;
import java.io.PrintWriter;

public class PolygonStyle {

    public static final int FILL_NONE = 0;
    public static final int FILL_COLOR = 1;
    public static final int FILL_TEXTURED = 2;

    private boolean visible = true;
    private boolean visibleInRadar = true;
    private Color color;
    private Pixmap texture;
    private int fillStyle;
    private LineStyle defEdgeStyle;
    private String id;


    public boolean isVisible () {
        return visible;
    }


    public void setVisible (boolean v) {
        this.visible = v;
    }


    public boolean isVisibleInRadar () {
        return visibleInRadar;
    }


    public void setVisibleInRadar (boolean v) {
        this.visibleInRadar = v;
    }

    
    public Color getColor () {
        return color;
    }


    public void setColor (Color c) {
        this.color = c;
    }


    public Pixmap getTexture () {
        return texture;
    }


    public void setTexture (Pixmap t) {
        this.texture = t;
    }


    public int getFillStyle () {
        return fillStyle;
    }


    public void setFillStyle (int f) {
        this.fillStyle = f;
    }


    public LineStyle getDefaultEdgeStyle () {
        return defEdgeStyle;
    }


    public void setDefaultEdgeStyle (LineStyle l) {
        this.defEdgeStyle = l;
    }


    public String getId () {
        return id;
    }

    
    public void setId (String id) {
        this.id = id;
    }


    public void printXml (PrintWriter out) throws IOException {

        out.print("<PolyStyle id=\"");
        out.print(id);

        if (fillStyle == FILL_COLOR && color != null) {
            out.print("\" color=\"");
            out.print(color.getRGB());
        } else if (fillStyle == FILL_TEXTURED && texture != null) {
            out.print("\" texture=\"");
            out.print(texture.getFileName());
        }
        out.print("\" defedge=\"");
        out.print(defEdgeStyle.getId());
        out.print("\" flags=\"");
        out.print(computeFlags());
        out.println("\"/>");
    }

    
    private int computeFlags () {
        /* TODO: implement */
        return 1;
    }
}

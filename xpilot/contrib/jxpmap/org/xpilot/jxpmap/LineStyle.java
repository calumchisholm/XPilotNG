package org.xpilot.jxpmap;

import java.awt.*;
import java.io.PrintWriter;
import java.io.IOException;

public class LineStyle {

    public static final int STYLE_SOLID = 0;
    public static final int STYLE_ONOFFDASH = 1;
    public static final int STYLE_DOUBLEDASH = 2;
    public static final int STYLE_HIDDEN = 3;

    private Color color;
    private Stroke stroke;
    private int style;
    private String id;    
    private int width;
    private float cscale;

    
    public LineStyle () {
        color = Color.black;
        width = 1;
    }


    public LineStyle (String id, int width, Color color, int style) {
        this.id = id;
        this.color = color;
        this.width = width;
        this.style = style;
    }


    public int getStyle () {
        return style;
    }

    
    public void setStyle (int style) {
        this.style = style;
        stroke = null;
    }


    public Stroke getStroke (float scale) {
        if (stroke == null || cscale != scale) {
            if (style == STYLE_SOLID) {
                stroke = new BasicStroke(width / scale,
                                         BasicStroke.CAP_BUTT, 
                                         BasicStroke.JOIN_MITER);
            } else if (style != STYLE_HIDDEN) {
                float dash[] = { 10 / scale };
                stroke = new BasicStroke(width / scale,
                                         BasicStroke.CAP_BUTT, 
                                         BasicStroke.JOIN_MITER, 
                                         10 / scale, dash, 0.0f);
            }
            cscale = scale;
        }
        return stroke;
    }


    public Color getColor () {
        return color;
    }


    public void setColor (Color c) {
        this.color = c;
    }

    
    public String getId () {
        return id;
    }

    
    public void setId (String id) {
        this.id = id;
    }


    public int getWidth () {
        return width;
    }


    public void setWidth (int width) {
        this.width = width;
        stroke = null;
    }


    public void printXml (PrintWriter out) throws IOException {
	if (id == "internal") return;
        out.print("<EdgeStyle id=\"");
        out.print(id);
        out.print("\" width=\"");
        if (style == STYLE_HIDDEN) out.print(-1);
        else out.print(width);
        out.print("\" color=\"");
        out.print(PolygonStyle.toRgb(color));
        out.print("\" style=\"");
        if (style == STYLE_HIDDEN) out.print(STYLE_SOLID);
        else out.print(style);
        out.println("\"/>");

    }
}

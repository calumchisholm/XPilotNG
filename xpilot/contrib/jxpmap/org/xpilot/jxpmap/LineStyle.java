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

    public LineStyle (String id, int width, Color color, int style) {
        this.id = id;
        this.color = color;
        this.width = width;
        if (style == STYLE_SOLID) {
            stroke = new BasicStroke((float)width, 
                                     BasicStroke.CAP_BUTT, 
                                     BasicStroke.JOIN_MITER);
        } else if (style != STYLE_HIDDEN) {
            float dash[] = { 10.0f };
            stroke = new BasicStroke((float)width, 
                                     BasicStroke.CAP_BUTT, 
                                     BasicStroke.JOIN_MITER, 
                                     10.0f, dash, 0.0f);
        }            
    }


    public int getStyle () {
        return style;
    }


    public Stroke getStroke() {
        return stroke;
    }


    public Color getColor () {
        return color;
    }

    
    public String getId () {
        return id;
    }

    
    public void setId (String id) {
        this.id = id;
    }    


    public void printXml (PrintWriter out) throws IOException {
        out.print("<EdgeStyle id=\"");
        out.print(id);
        out.print("\" width=\"");
        out.print(width);
        out.print("\" color=\"");
        out.print(color.getRGB());
        out.print("\" style=\"");
        out.print(style);
        out.println("\"/>");

    }
}

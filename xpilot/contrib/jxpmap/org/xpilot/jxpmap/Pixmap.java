package org.xpilot.jxpmap;

import java.awt.image.BufferedImage;
import java.io.*;
import java.util.Map;

public class Pixmap extends ModelObject {
    
    private String fileName;
    private boolean scalable;
    private BufferedImage image;

    public Pixmap () {
        this.scalable = true;
    }

    
    public String getFileName () {
        return fileName;
    }
    
    public void setFileName (String  f) {
        this.fileName = f;
    }
        
    public boolean isScalable () {
        return scalable;
    }
    
    public void setScalable (boolean s) {
        this.scalable = s;
    }
    
    
    public BufferedImage getImage () {
        return image;
    }
    
    
    public void load () throws IOException {
        image = 
            new PPMDecoder().decode
                (new BufferedInputStream
                    (new FileInputStream(fileName)));
    }


    public void printXml (PrintWriter out) throws IOException {
        
        out.print("<BmpStyle id=\"");
        out.print(getFileName());
        out.print("\" filename=\"");
        out.print(getFileName());
        out.println("\" flags=\"1\"/>");
    }
}

package org.xpilot.jxpmap;

import java.awt.image.BufferedImage;
import java.io.*;

public class Pixmap {
    
    private String fileName;
    private boolean scalable;
    private BufferedImage image;
    private BufferedImage scaledImage;
    private float currentScale;

    
    public Pixmap () {
        this.currentScale = 1.0f;
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
        return scaledImage;
    }
    
    
    public void load () throws IOException {
        scaledImage = image = 
            new PPMDecoder().decode
                (new BufferedInputStream
                    (new FileInputStream(fileName)));
    }


    public void scale (float scaleFactor) {
        if (currentScale == scaleFactor) return;
        // TODO: implement properly if needed anymore
    }
}

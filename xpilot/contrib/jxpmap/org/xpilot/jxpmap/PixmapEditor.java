package org.xpilot.jxpmap;

import javax.swing.*;
import java.awt.*;

public class PixmapEditor extends EditorPanel {

    private Pixmap pixmap;
    private JTextField nameField;
    

    public PixmapEditor (Pixmap pixmap) {
        setTitle("Image Properties");
        this.pixmap = pixmap;
        setLayout(new BorderLayout());
        add(new JLabel("Name"), BorderLayout.WEST);
        add(nameField = new JTextField(), BorderLayout.CENTER);
        if (pixmap.getFileName() != null) 
            nameField.setText(pixmap.getFileName());
    }

    
    public boolean apply () {
        pixmap.setFileName(nameField.getText());
        pixmap.setScalable(true);
        try {
            pixmap.load();
            return true;
        } catch (Exception e) {
            JOptionPane.showMessageDialog
                (this, "Failed to load the specified image. Check the name.",
                 "Error", JOptionPane.ERROR_MESSAGE);
            return false;
        }
    }


    public boolean cancel () {
        pixmap.setFileName(null);
        return true;
    }
}

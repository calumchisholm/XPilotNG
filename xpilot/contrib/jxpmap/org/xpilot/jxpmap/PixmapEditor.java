package org.xpilot.jxpmap;

import java.awt.BorderLayout;

import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

public class PixmapEditor extends EditorPanel {

    private Pixmap pixmap;
    private JTextField nameField;
    

    public PixmapEditor (Pixmap pixmap) {
        setTitle("Image");
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

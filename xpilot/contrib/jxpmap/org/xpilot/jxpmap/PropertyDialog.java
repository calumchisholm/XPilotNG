package org.xpilot.jxpmap;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class PropertyDialog extends JDialog implements ActionListener {

    
    private JButton btnOk;
    private JButton btnCancel;
    private PropertyEditor editor;
    

    protected PropertyDialog (Component parent, 
                              PropertyEditor editor, 
                              boolean modal) {

        super(JOptionPane.getFrameForComponent(parent), modal);
        this.editor = editor;
        
        setTitle("Editor Dialog");

        btnOk = new JButton("OK");
        btnOk.addActionListener(this);
        btnCancel = new JButton("Cancel");
        btnCancel.addActionListener(this);

        JPanel p1 = new JPanel(new GridLayout(1,2));
        p1.add(btnOk);
        p1.add(btnCancel);

        JPanel p2 = new JPanel();
        p2.add(p1);

        getContentPane().add(p2, BorderLayout.SOUTH);

        editor.setBorder
            (BorderFactory.createCompoundBorder
             (BorderFactory.createEtchedBorder(),
              BorderFactory.createEmptyBorder(5, 5, 5, 5)));
        
        getContentPane().add(editor, BorderLayout.CENTER);
    }


    protected PropertyDialog (MapCanvas canvas, MapObject object) {
        this(canvas, object.getPropertyEditor(canvas), true);
    }


    public static void show (MapCanvas canvas, MapObject object) {
        
        PropertyDialog dialog = new PropertyDialog(canvas, object);
        dialog.pack();
        dialog.setLocationRelativeTo(JOptionPane.getFrameForComponent(canvas));
        dialog.setVisible(true);
    }


    public static void show (Component parent, 
                             PropertyEditor editor, 
                             boolean model) {

        PropertyDialog dialog = new PropertyDialog(parent, editor, model);
        dialog.pack();
        dialog.setLocationRelativeTo(JOptionPane.getFrameForComponent(parent));
        dialog.setVisible(true);
    }


    public void actionPerformed (ActionEvent evt) {

        if (evt.getSource() == btnOk) {
            if (!editor.apply()) return;
            
        } else if (evt.getSource() == btnCancel) {
            if (!editor.cancel()) return;
            
        } else {
            return;
        }
        
        dispose();
    }
}

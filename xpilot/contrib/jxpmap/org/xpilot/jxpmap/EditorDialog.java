package org.xpilot.jxpmap;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class EditorDialog extends JDialog implements ActionListener {


    private JButton btnApply;
    private JButton btnOk;
    private JButton btnCancel;
    private EditorPanel editor;
    

    protected EditorDialog (Component parent, 
                            EditorPanel editor, 
                            boolean modal) {

        super(JOptionPane.getFrameForComponent(parent), modal);
        this.editor = editor;
        
        setTitle(editor.getTitle());
        JPanel p1 = null;
        
        if (!modal) {
            p1 = new JPanel(new GridLayout(1,3));
            btnApply = new JButton("Apply");
            btnApply.addActionListener(this);
            p1.add(btnApply);
        } else {
            p1 = new JPanel(new GridLayout(1,2));
        }

        btnOk = new JButton("OK");
        btnOk.addActionListener(this);
        p1.add(btnOk);
        
        btnCancel = new JButton("Cancel");
        btnCancel.addActionListener(this);
        p1.add(btnCancel);

        JPanel p2 = new JPanel();
        p2.add(p1);

        getContentPane().add(p2, BorderLayout.SOUTH);

        editor.setBorder
            (BorderFactory.createCompoundBorder
             (BorderFactory.createRaisedBevelBorder(),
              BorderFactory.createEmptyBorder(5, 5, 5, 5)));
        
        getContentPane().add(editor, BorderLayout.CENTER);
    }


    protected EditorDialog (MapCanvas canvas, MapObject object) {
        this(canvas, object.getPropertyEditor(canvas), true);
    }


    public static void show (MapCanvas canvas, MapObject object) {
        
        EditorDialog dialog = new EditorDialog(canvas, object);
        dialog.pack();
        dialog.setLocationRelativeTo(JOptionPane.getFrameForComponent(canvas));
        dialog.setVisible(true);
    }


    public static void show (Component parent, 
                             EditorPanel editor, 
                             boolean model) {

        EditorDialog dialog = new EditorDialog(parent, editor, model);
        dialog.pack();
        dialog.setLocationRelativeTo(JOptionPane.getFrameForComponent(parent));
        dialog.setVisible(true);
    }


    public void actionPerformed (ActionEvent evt) {

        if (evt.getSource() == btnApply) {
            editor.apply();
            return;
            
        } else if (evt.getSource() == btnOk) {
            if (!editor.apply()) return;
            
        } else if (evt.getSource() == btnCancel) {
            if (!editor.cancel()) return;
            
        } else {
            return;
        }
        
        dispose();
    }
}

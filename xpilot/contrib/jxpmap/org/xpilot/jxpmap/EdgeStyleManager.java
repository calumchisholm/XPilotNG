package org.xpilot.jxpmap;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;
import java.awt.event.*;
import java.util.List;
import java.util.ArrayList;
import java.awt.Image;
import java.awt.GridLayout;
import java.awt.BorderLayout;

public class EdgeStyleManager extends EditorPanel
    implements ListSelectionListener, ActionListener {
    
    private EdgeStylePreview preview;
    private DefaultListModel jlistModel;
    private JList jlist;
    private JButton btnAdd;
    private JButton btnEdit;
    private JButton btnDel;
    private List styles;
    private MapCanvas canvas;
    
    
    public EdgeStyleManager (MapCanvas canvas) {
        
        setTitle("Edge Style Manager");
        
        this.canvas = canvas;
        this.styles = canvas.getModel().edgeStyles;
        
        jlistModel = new DefaultListModel();
        for (int i = 0; i < styles.size(); i++) 
            jlistModel.addElement(((LineStyle)styles.get(i)).getId());
        jlist = new JList(jlistModel);
        jlist.addListSelectionListener(this);
        jlist.setFixedCellWidth(100);
        JScrollPane sp = new JScrollPane(jlist);

        setLayout(new GridLayout(1,2));
        JPanel p1 = new JPanel(new BorderLayout(5, 5));
        JPanel p2 = new JPanel();
        JPanel p3 = new JPanel(new GridLayout(1,3));
        
        btnAdd = new JButton("Add");
        btnAdd.addActionListener(this);
        btnEdit = new JButton("Edit");
        btnEdit.addActionListener(this);
        btnDel = new JButton("Del");
        btnDel.addActionListener(this);

        p3.add(btnAdd);
        p3.add(btnEdit);
        p3.add(btnDel);

        p2.add(p3);
        p1.add(p2, BorderLayout.SOUTH);
        p1.add(sp, BorderLayout.CENTER);
        add(p1);

        preview = new EdgeStylePreview();
        JPanel p4 = new JPanel(new BorderLayout());
        p4.setBorder(BorderFactory.createLoweredBevelBorder());
        p4.add(preview);
        add(p4);
    }


    private void showPreview (LineStyle style) {
        preview.setStyle(style);
    }
    
    
    public void valueChanged (ListSelectionEvent evt) {
        int index = jlist.getSelectedIndex();
        if (index == -1) return;
        showPreview((LineStyle)styles.get(index));
    }
    
    
    public void actionPerformed (ActionEvent evt) {
        Object src = evt.getSource();

        if (src == btnAdd) {

            LineStyle style = new LineStyle();
            EditorDialog.show
                (this, new EdgeStyleEditor(style, canvas.getModel()), true);
            if (style.getId() != null) {
                styles.add(style);
                jlistModel.addElement(style.getId());
            }
            return;
            
            
        } else if (src == btnEdit) {

            int index = jlist.getSelectedIndex();
            if (index == -1) {
                JOptionPane.showMessageDialog
                    (this, "First select a style from the list", 
                     "Information", JOptionPane.INFORMATION_MESSAGE);
                return;
            }
            LineStyle style = (LineStyle)styles.get(index);
            EditorDialog.show
                (this, new EdgeStyleEditor(style, canvas.getModel()), true);
            jlistModel.set(index, style.getId());
            canvas.repaint();
            preview.repaint();
            return;


        } else if (src == btnDel) {

            int index = jlist.getSelectedIndex();
            if (index == -1) {
                JOptionPane.showMessageDialog
                    (this, "First select a style from the list", 
                     "Information", JOptionPane.INFORMATION_MESSAGE);
                return;
            }
            styles.remove(index);
            jlistModel.remove(index);
            return;
        }
    }
}

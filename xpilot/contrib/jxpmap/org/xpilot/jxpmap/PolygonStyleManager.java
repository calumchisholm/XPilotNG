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

public class PolygonStyleManager extends EditorPanel
    implements ListSelectionListener, ActionListener {

    private PolygonStylePreview preview;
    private DefaultListModel jlistModel;
    private JList jlist;
    private JButton btnAdd;
    private JButton btnEdit;
    private JButton btnDel;
    private List myStyles;
    private List origStyles;
    private MapModel model;
    private MapCanvas canvas;
    
    
    public PolygonStyleManager (MapCanvas canvas) {
        
        setTitle("Polygon Style Manager");

        this.canvas = canvas;
        this.model = canvas.getModel();
        this.origStyles = model.polyStyles;
        myStyles = new ArrayList();
        myStyles.addAll(origStyles);
        
        jlistModel = new DefaultListModel();
        for (int i = 0; i < origStyles.size(); i++) 
            jlistModel.addElement(((PolygonStyle)myStyles.get(i)).getId());
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

        preview = new PolygonStylePreview();
        JPanel p4 = new JPanel(new BorderLayout());
        p4.setBorder(BorderFactory.createLoweredBevelBorder());
        p4.add(preview);
        add(p4);
    }


    private void showPreview (PolygonStyle style) {
        preview.setStyle(style);
    }
    
    
    public void valueChanged (ListSelectionEvent evt) {
        int index = jlist.getSelectedIndex();
        if (index == -1) return;
        showPreview((PolygonStyle)myStyles.get(index));
    }
    
    
    public void actionPerformed (ActionEvent evt) {
        Object src = evt.getSource();

        if (src == btnAdd) {

            PolygonStyle style = new PolygonStyle();
            style.setDefaultEdgeStyle(model.getDefaultEdgeStyle());
            EditorDialog.show
                (this, new PolygonStyleEditor(style, model), true);
            if (style.getId() != null) {
                myStyles.add(style);
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
            PolygonStyle style = (PolygonStyle)myStyles.get(index);
            EditorDialog.show
                (this, new PolygonStyleEditor(style, model), true);
            jlistModel.set(index, style.getId());
            return;


        } else if (src == btnDel) {

            int index = jlist.getSelectedIndex();
            if (index == -1) {
                JOptionPane.showMessageDialog
                    (this, "First select a style from the list", 
                     "Information", JOptionPane.INFORMATION_MESSAGE);
                return;
            }
            myStyles.remove(index);
            jlistModel.remove(index);
            return;
        }
    }


    public boolean apply () {
        origStyles.clear();
        origStyles.addAll(myStyles);
        canvas.repaint();
        preview.repaint();
        return true;
    }
}

package org.xpilot.jxpmap;

import java.awt.*;
import java.util.*;
import javax.swing.*;
import javax.swing.table.*;


public class MapOptionEditor extends PropertyEditor {

    private DefaultTableModel tableModel;
    private MapOptions options;

    public MapOptionEditor (MapOptions options) {
        this.options = options;
        tableModel = createTableModel(options);
        JTable table = new JTable(tableModel);
        JScrollPane pane = new JScrollPane(table);
        add(pane);
    }


    private DefaultTableModel createTableModel (MapOptions ops) {

        ArrayList rows = new ArrayList(ops.entrySet());
        Collections.sort(rows, new Comparator () {
                public int compare (Object o1, Object o2) {
                    return 
                        ((String)((Map.Entry)o1).getKey()).compareTo
                        ((String)((Map.Entry)o2).getKey());
                }
            });
        
        Object data[][] = new Object[rows.size()][2];
        for (int i = 0; i < rows.size(); i++) {
            Map.Entry entry = (Map.Entry)rows.get(i);
            data[i][0] = entry.getKey();
            data[i][1] = entry.getValue();
        }

        Object cols[] = new Object[] { "Option", "Value" };

        return new DefaultTableModel(data, cols);
    }


    public boolean apply () {
        Vector v = tableModel.getDataVector();
        options.clear();
        for (int i = 0; i < v.size(); i++) {
            Vector row = (Vector)v.get(i);
            options.put(row.get(0), row.get(1));
        }
        options.updated();
        return true;
    }
}

package org.xpilot.jxpmap;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.File;

public class MainFrame extends JFrame implements ActionListener {

    private JScrollPane sp;
    private MapCanvas canvas;
    private int zoom;

    private JToggleButton btnNewPoly;
    private JToggleButton btnNewFuel;
    private JToggleButton btnNewBase;
    private JToggleButton btnNewBall;
    private JToggleButton btnNewCheckPoint;
    private JToggleButton btnErase;
    private JButton btnZoomIn;
    private JButton btnZoomOut;

    public MainFrame () {

        super("jXPMap Editor");
        canvas = new MapCanvas();
        sp = new JScrollPane(canvas);
        getContentPane().add(sp, BorderLayout.CENTER);
        buildMenuBar();
        buildToolBar();
        setSize(500, 400);
        zoom = 0;
    }


    public void setModel (MapModel model) {
        canvas.setModel(model);
    }


    private void buildMenuBar () {

        JMenu menu;
        JMenuItem menuItem;
        JMenuBar menuBar = new JMenuBar();
        setJMenuBar(menuBar);

        menu = new JMenu("File");
        menuBar.add(menu);

        menuItem = new JMenuItem("New");
        menu.add(menuItem);
        menuItem.setActionCommand("newMap");
        menuItem.addActionListener(this);

        menuItem = new JMenuItem("Open");
        menu.add(menuItem);
        menuItem.setActionCommand("openMap");
        menuItem.addActionListener(this);

        menuItem = new JMenuItem("Save");
        menu.add(menuItem);
        menuItem.setActionCommand("saveMap");
        menuItem.addActionListener(this);

        menuItem = new JMenuItem("Exit");
        menu.add(menuItem);
        menuItem.setActionCommand("exitApp");
        menuItem.addActionListener(this);


        menu = new JMenu("View");
        menuBar.add(menu);

        menuItem = new JMenuItem("Polygon styles");
        menu.add(menuItem);
        menuItem.setActionCommand("showPolygonStyles");
        menuItem.addActionListener(this);

        menuItem = new JMenuItem("Edge styles");
        menu.add(menuItem);
        menuItem.setActionCommand("showEdgeStyles");
        menuItem.addActionListener(this);

        menuItem = new JMenuItem("Images");
        menu.add(menuItem);
        menuItem.setActionCommand("showImages");
        menuItem.addActionListener(this);

        menuItem = new JMenuItem("Options");
        menu.add(menuItem);
        menuItem.setActionCommand("showOptions");
        menuItem.addActionListener(this);
    }


    private void buildToolBar () {


        JToolBar toolBar = new JToolBar(SwingConstants.VERTICAL);

        btnNewPoly = createToggle
            ("newPolygon", "/images/polyicon.gif", "New polygon");
        toolBar.add(btnNewPoly);

        btnNewFuel = createToggle
            ("newFuel", "/images/fuelicon.gif", "New fuel station");
        toolBar.add(btnNewFuel);

        btnNewBase = createToggle
            ("newBase", "/images/baseicon.gif", "New base");
        toolBar.add(btnNewBase);

        btnNewBall = createToggle
            ("newBall", "/images/ballicon.gif", "New ball");
        toolBar.add(btnNewBall);

        btnNewCheckPoint = createToggle
            ("newCheckPoint", "/images/checkicon.gif", "New checkpoint");
        toolBar.add(btnNewCheckPoint);

        btnErase = createToggle
            ("toggleEraseMode", "/images/eraseicon.gif", "Erasing mode");
        toolBar.add(btnErase);

        toolBar.addSeparator();

        btnZoomIn = createButton
            ("zoomIn", "/images/zoominicon.gif", "Zoom in");
        toolBar.add(btnZoomIn);

        btnZoomOut = createButton
            ("zoomOut", "/images/zoomouticon.gif", "Zoom out");
        toolBar.add(btnZoomOut);

        getContentPane().add(toolBar, BorderLayout.WEST);
    }


    public void actionPerformed (ActionEvent ae) {
        String cmd = ae.getActionCommand();
        try {
            getClass().getDeclaredMethod(cmd, null).invoke(this, null);
        } catch (NoSuchMethodException nsme) {
            JOptionPane.showMessageDialog
                (this, "Sorry, operation "+ ae.getActionCommand() + 
                 " is not implemented yet", "Error",
                 JOptionPane.ERROR_MESSAGE);
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog
                (this, "Unexpected exception: " + e, "Error",
                 JOptionPane.ERROR_MESSAGE);
        }
    }
    

    private JToggleButton createToggle (String cmd, 
                                        String name, 
                                        String toolTip) {
        JToggleButton b = new JToggleButton
            (new ImageIcon(getClass().getResource(name)));
        b.setToolTipText(toolTip);
        b.setActionCommand(cmd);
        b.addActionListener(this);
        b.setMargin(new Insets(1, 1, 1, 1));
        return b;
    }


    private JButton createButton (String cmd, String name, String toolTip) {
        JButton b = new JButton(new ImageIcon(getClass().getResource(name)));
        b.setToolTipText(toolTip);
        b.setActionCommand(cmd);
        b.addActionListener(this);
        return b;
    }


    private void zoomIn () {
        zoom++;
        updateScale();
    }


    private void zoomOut () {
        zoom--;
        updateScale();
    }


    private void updateScale () {
        float df = canvas.getModel().getDefaultScale();
        canvas.setScale
            ((zoom >= 0) ?
             (df * (zoom + 1)) :
             (df / (-zoom + 1)));
        canvas.repaint();
    }

    
    private void newPolygon () {
        
        if (!btnNewPoly.isSelected()) {
            canvas.repaint();
            canvas.setCanvasEventHandler(null);
            return;
        }

        updateToggles(btnNewPoly);
        
        canvas.setErase(false);
        canvas.setCanvasEventHandler(new CanvasEventAdapter () {
                public void mousePressed (MouseEvent me) {
                    CanvasEventHandler ceh = 
                        new MapPolygon().getCreateHandler
                            (new ToggleCommand(btnNewPoly));

                    canvas.setCanvasEventHandler(ceh);
                    ceh.mousePressed(me);
                }
            });
    }
    
    
    private void newFuel () {

        if (!btnNewFuel.isSelected()) {
            canvas.repaint();
            canvas.setCanvasEventHandler(null);
            return;
        }

        updateToggles(btnNewFuel);

        canvas.setErase(false);
        canvas.setCanvasEventHandler
            (new MapFuel().getCreateHandler
                (new ToggleCommand(btnNewFuel)));
    }


    private void newBase () {

        if (!btnNewBase.isSelected()) {
            canvas.repaint();
            canvas.setCanvasEventHandler(null);
            return;
        }

        updateToggles(btnNewBase);

        canvas.setErase(false);
        canvas.setCanvasEventHandler
            (new MapBase().getCreateHandler
                (new ToggleCommand(btnNewBase)));
    }


    private void newBall () {

        if (!btnNewBall.isSelected()) {
            canvas.repaint();
            canvas.setCanvasEventHandler(null);
            return;
        }

        updateToggles(btnNewBall);

        canvas.setErase(false);
        canvas.setCanvasEventHandler
            (new MapBall().getCreateHandler
                (new ToggleCommand(btnNewBall)));
    }


    private void newCheckPoint () {

        if (!btnNewCheckPoint.isSelected()) {
            canvas.repaint();
            canvas.setCanvasEventHandler(null);
            return;
        }

        updateToggles(btnNewCheckPoint);

        canvas.setErase(false);
        canvas.setCanvasEventHandler
            (new MapCheckPoint().getCreateHandler
                (new ToggleCommand(btnNewCheckPoint)));
    }


    private void toggleEraseMode () {
        updateToggles(btnErase);
        canvas.setCanvasEventHandler(null);
        canvas.setErase(!canvas.isErase());
        canvas.repaint();
    }


    private void newMap () {
        setModel(new MapModel());
        showOptions();
    }

    
    private void openMap () {
        
        JFileChooser fc = new JFileChooser();
        int rv = fc.showOpenDialog(this);
        if (rv != JFileChooser.APPROVE_OPTION) return;
        
        File f = fc.getSelectedFile();
        if (f == null) return;
        
        MapModel model = new MapModel();
        try {
            model.load(f.getAbsolutePath());
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog
                (this, "Loading failed: " + e.getMessage(), "Error", 
                 JOptionPane.ERROR_MESSAGE);
            return;
        }
        setModel(model);
    }


    private void saveMap () {
        JFileChooser fc = new JFileChooser();
        int rv = fc.showSaveDialog(this);
        if (rv != JFileChooser.APPROVE_OPTION) return;

        File f = fc.getSelectedFile();
        if (f == null) return;
        
        try {
            canvas.getModel().save(f);
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog
                (this, "Saving failed: " + e.getMessage(), "Error",
                 JOptionPane.ERROR_MESSAGE);
        }
    }


    private void exitApp () {
        System.exit(0);
    }


    private void showOptions () {
        EditorDialog.show
            (this, 
             new MapOptionEditor(canvas.getModel().options), 
             false, 
             EditorDialog.CLOSE);
    }


    private void showImages () {
        EditorDialog.show
            (this, 
             new ImageListEditor(canvas.getModel().pixmaps), 
             false,
             EditorDialog.CLOSE);
    }


    private void showPolygonStyles () {
        EditorDialog.show
            (this, 
             new PolygonStyleManager(canvas), 
             false,
             EditorDialog.CLOSE);
    }


    private void showEdgeStyles () {
        EditorDialog.show
            (this, 
             new EdgeStyleManager(canvas), 
             false,
             EditorDialog.CLOSE);
    }


    private void updateToggles (JToggleButton boss) {
        if (boss != btnNewPoly) btnNewPoly.setSelected(false);
        if (boss != btnNewFuel) btnNewFuel.setSelected(false);
        if (boss != btnNewBase) btnNewBase.setSelected(false);
        if (boss != btnNewCheckPoint) btnNewCheckPoint.setSelected(false);
        if (boss != btnErase) btnErase.setSelected(false);
    }


    private class ToggleCommand implements Runnable {

        private JToggleButton b;

        public ToggleCommand (JToggleButton b) {
            this.b = b;
        }

        public void run () {
            b.setSelected(!b.isSelected());
        }
    }


    public static void main (String args[]) throws Exception {

        MainFrame mf = new MainFrame();
        mf.setVisible(true);

        if (args.length == 0) mf.newMap();
        else {
            MapModel model = new MapModel();
            if (args.length > 0) model.load(args[0]);
            mf.setModel(model);

        }
    }
}

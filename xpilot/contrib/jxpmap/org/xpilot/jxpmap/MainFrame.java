package org.xpilot.jxpmap;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.AbstractAction;
import javax.swing.ActionMap;
import javax.swing.ButtonGroup;
import javax.swing.ImageIcon;
import javax.swing.InputMap;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.SwingConstants;

public class MainFrame extends JFrame implements ActionListener {

    private MapCanvas canvas;
    private int zoom;
    
    private ButtonGroup toggleGroup;
    private JLabel lblZoom;
    private File mapFile;

    public MainFrame () {

        super("jXPMap Editor");
        canvas = new MapCanvas();
        getContentPane().add(canvas, BorderLayout.CENTER);
        buildMenuBar();
        buildToolBar();
        buildActionMap();
        buildInputMap();
        setSize(800, 600);
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        Dimension ss = Toolkit.getDefaultToolkit().getScreenSize();
        setLocation(ss.width / 2 - 400, ss.height / 2 - 300);
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


        menu = new JMenu("Edit");
        menuBar.add(menu);
        menuItem = new JMenuItem("Undo");
        menu.add(menuItem);
        menuItem.setActionCommand("undo");
        menuItem.addActionListener(this);

        menuItem = new JMenuItem("Redo");
        menu.add(menuItem);
        menuItem.setActionCommand("redo");
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

        JToolBar tb1 = new JToolBar(SwingConstants.HORIZONTAL);
        lblZoom = new JLabel("x1");
        lblZoom.setHorizontalAlignment(SwingConstants.CENTER);
        Font f = lblZoom.getFont();
        lblZoom.setFont(f.deriveFont((float)(f.getSize() - 2)));        
        toggleGroup = new ButtonGroup();

        tb1.add(newToggle("newWall", "/images/polyicon.gif", "New wall"));
        tb1.add(newToggle("newBallArea", "/images/ballareaicon.gif", "New ball area"));        
        tb1.add(newToggle("newBallTarget", "/images/balltargeticon.gif", "New ball target"));
        tb1.add(newToggle("newDecor", "/images/decoricon.gif", "New decoration"));
        tb1.add(newToggle("newTarget", "/images/targeticon.gif", "New target"));
        tb1.add(newToggle("newCannon", "/images/cannonicon.gif", "New cannon"));        
        tb1.add(newToggle("newFuel", "/images/fuelicon.gif", "New fuel station"));
        tb1.add(newToggle("newBase", "/images/baseicon.gif", "New base"));
        tb1.add(newToggle("newBall", "/images/ballicon.gif", "New ball"));
        tb1.add(newToggle("newCheckPoint", "/images/checkicon.gif", "New checkpoint"));        
        tb1.addSeparator();
        tb1.add(newToggle("select", "/images/arrow.gif", "Select"));        
        tb1.add(newToggle("eraseMode", "/images/eraseicon.gif", "Erasing mode"));
        tb1.add(newToggle("copyMode", "/images/copyicon.gif", "Copy mode"));        
        tb1.addSeparator();
        tb1.add(newButton("zoomIn", "/images/zoominicon.gif", "Zoom in"));
        tb1.add(newButton("zoomOut", "/images/zoomouticon.gif", "Zoom out"));
        tb1.add(lblZoom);
        tb1.addSeparator();
        tb1.add(newButton("undo", "/images/undo.gif", "Undo"));
        tb1.add(newButton("redo", "/images/redo.gif", "Redo"));

        getContentPane().add(tb1, BorderLayout.NORTH);
    }


    private void buildActionMap () {
        ActionMap am = canvas.getActionMap();
        am.put("quickSave", new GuiAction("quickSave"));
        am.put("quickOpen", new GuiAction("quickOpen"));
    }


    private void buildInputMap () {
        InputMap im = canvas.getInputMap(
            MapCanvas.WHEN_IN_FOCUSED_WINDOW);
        im.put(KeyStroke.getKeyStroke("control S"), "quickSave");
        im.put(KeyStroke.getKeyStroke("control L"), "quickOpen");
        im.put(KeyStroke.getKeyStroke("control Z"), "undo");
    }


    public void actionPerformed (ActionEvent ae) {
        String cmd = ae.getActionCommand();
        dispatchCommand(cmd);
    }


    private void dispatchCommand (String cmd) {
        try {
            getClass().getDeclaredMethod(cmd, null).invoke(this, null);
        } catch (NoSuchMethodException nsme) {
            JOptionPane.showMessageDialog
                (this, "Sorry, operation " + cmd + 
                 " is not implemented yet", "Error",
                 JOptionPane.ERROR_MESSAGE);
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog
                (this, "Unexpected exception: " + e, "Error",
                 JOptionPane.ERROR_MESSAGE);
        }
    }
    

    private JToggleButton newToggle(String cmd, String name, String toolTip) {
        JToggleButton b =
            new JToggleButton(new ImageIcon(getClass().getResource(name)));
        b.setToolTipText(toolTip);
        b.setActionCommand(cmd);
        b.addActionListener(this);
        b.setPreferredSize(new Dimension(26, 26));
        toggleGroup.add(b);
        return b;
    }


    private JButton newButton(String cmd, String name, String toolTip) {
        JButton b = new JButton(new ImageIcon(getClass().getResource(name)));
        b.setToolTipText(toolTip);
        b.setActionCommand(cmd);
        b.setPreferredSize(new Dimension(26, 26));
        b.addActionListener(this);
        return b;
    }

    private void setZoom (int zoom) {
        this.zoom = zoom;
        updateScale();
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
        
        if (zoom >= 0) {
            lblZoom.setText("x" + (zoom + 1));
        } else {
            lblZoom.setText("x1/" + (-zoom + 1));
        }
    }
    
    private void select () {
        canvas.setErase(false);
        canvas.setCopy(false);
        canvas.setCanvasEventHandler(null);
        canvas.repaint();
    }

    private void newWall () {
        newMapObject("newWall", new MapPolygon());
    }
    
    private void newBallArea () {
        newMapObject("newBallArea", new BallArea());
    }

    private void newBallTarget () {
        newMapObject("newBallTarget", new BallTarget());
    }
    
    private void newDecor () {
        newMapObject("newDecor", new Decoration());
    }
    
    private void newTarget () {
        newMapObject("newTarget", new Target());
    }    
    
    private void newFuel () {
        newMapObject("newFuel", new Fuel());
    }

    private void newBase () {
        newMapObject("newBase", new Base());
    }

    private void newBall () {
        newMapObject("newBall", new Ball());    
    }

    private void newCannon () {
        newMapObject("newCannon", new Cannon());
    }

    private void newCheckPoint () {
        newMapObject("newCheckPoint", new CheckPoint());
    }
    
    private void newMapObject (String cmd, MapObject o) {
        canvas.setErase(false);
        canvas.setCopy(false);
        canvas.setCanvasEventHandler(o.getCreateHandler(null));        
    } 


    private void eraseMode () {
        canvas.setCanvasEventHandler(null);
        canvas.setErase(true);
        canvas.repaint();
    }
    
    private void copyMode () {
        canvas.setCanvasEventHandler(null);
        canvas.setCopy(true);
        canvas.repaint();
    }


    private void newMap () {
        mapFile = null;
        setModel(new MapModel());
        //setZoom(-5);
    }

    
    private void openMap () {
        
        JFileChooser fc = new JFileChooser();
        if (mapFile != null) fc.setSelectedFile(mapFile);
        int rv = fc.showOpenDialog(this);
        if (rv != JFileChooser.APPROVE_OPTION) return;
        
        File f = fc.getSelectedFile();
        if (f == null) return;
        mapFile = f;
        
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
        //setZoom(-5);
    }


    private void saveMap () {
        JFileChooser fc = new JFileChooser(mapFile);
        if (mapFile != null) fc.setSelectedFile(mapFile);
        int rv = fc.showSaveDialog(this);
        if (rv != JFileChooser.APPROVE_OPTION) return;

        File f = fc.getSelectedFile();
        if (f == null) return;
        mapFile = f;
        
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


    private void quickSave () {

        if (mapFile == null) {
            saveMap();
            return;
        }
        try {
            canvas.getModel().save(mapFile);
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog
                (this, "Saving failed: " + e.getMessage(), "Error",
                 JOptionPane.ERROR_MESSAGE);
        }
    }


    private void quickOpen () {

        if (mapFile == null) {
            openMap();
            return;
        }
        MapModel model = new MapModel();
        try {
            model.load(mapFile.getAbsolutePath());
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog
                (this, "Loading failed: " + e.getMessage(), "Error", 
                 JOptionPane.ERROR_MESSAGE);
            return;
        }
        setModel(model);
    }

    private void undo () {
        if (canvas.getUndoManager().canUndo())
            canvas.getUndoManager().undo();
        canvas.repaint();            
    }

    private void redo () {
        if (canvas.getUndoManager().canRedo())
            canvas.getUndoManager().redo();
        canvas.repaint();            
    }


    private class GuiAction extends AbstractAction {

        private String cmd;

        public GuiAction (String cmd) {
            super();
            this.cmd = cmd;
        }

        public void actionPerformed (ActionEvent ae) {
            MainFrame.this.dispatchCommand(cmd);
        }
    }


    public static void main (String args[]) throws Exception {

        MainFrame mf = new MainFrame();
        mf.setVisible(true);

        if (args.length == 0) mf.newMap();
        else {
            mf.mapFile = new File(args[0]);
            MapModel model = new MapModel();
            if (args.length > 0) model.load(args[0]);
            mf.setModel(model);
            //mf.setZoom(-5);
        }
    }
}

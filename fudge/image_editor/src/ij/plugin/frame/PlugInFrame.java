package ij.plugin.frame;
import java.awt.*;
import java.awt.event.*;
import ij.*;
import ij.plugin.*;
import javax.swing.*;

/**  This is a closeable window that plugins can extend. */
public class PlugInFrame extends JFrame implements PlugIn, WindowListener, FocusListener {

	String title;
	
	public PlugInFrame(String title) {
		super(title);
		enableEvents(AWTEvent.WINDOW_EVENT_MASK);
		this.title = title;
		ImageJ ij = IJ.getInstance();
		addWindowListener(this);
 		addFocusListener(this);
		//setBackground(Color.white);
		if (IJ.debugMode) IJ.log("opening "+title);
		setVisible(true);
	}
	
	public void run(String arg) {
	}
	
    public void windowClosing(WindowEvent e) {
    	if (e.getSource()==this)
    		close();
    }
    
    /** Closes this window. */
    public void close() {
		setVisible(false);
		dispose();
    }

    public void windowActivated(WindowEvent e) {
		if (IJ.isMacintosh() && IJ.getInstance()!=null) {
			IJ.wait(1); // needed for 1.4.1 on OS X
			setJMenuBar(Menus.getMenuBar());
		}
	}

	public void focusGained(FocusEvent e) {
		//IJ.log("PlugInFrame: focusGained");
	}

    public void windowOpened(WindowEvent e) {}
    public void windowClosed(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowDeactivated(WindowEvent e) {}
	public void focusLost(FocusEvent e) {}
}
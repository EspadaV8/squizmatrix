/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: AssetMap.java,v 1.12 2005/03/06 22:13:26 mmcintyre Exp $
*
*/

package net.squiz.matrix.assetmap;

import javax.swing.*;
import javax.swing.tree.*;
import net.squiz.matrix.ui.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.plaf.*;
import java.util.*;
import java.awt.event.*;
import java.awt.*;
import java.net.*;
import netscape.javascript.*;
import javax.swing.plaf.metal.*;
import javax.swing.border.*;
import net.squiz.matrix.debug.*;
import java.io.IOException;

/**
 * The main applet class
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetMap extends JApplet implements InitialisationListener {

	private BasicView view1;
	private BasicView view2;
	private MatrixTabbedPane pane;
	protected static JSObject window;
	public static AssetMap applet;
	private javax.swing.Timer timer;
	public static final int POLLING_TIME = 2000;
	
	/**
	 * Constructs the Asset Map
	 */
	public AssetMap() {
		try {
			UIManager.setLookAndFeel(new MatrixLookAndFeel());
		} catch (UnsupportedLookAndFeelException ulnfe) {
			ulnfe.printStackTrace();
		}
		applet = this;
	}

	// MM: find a better solution for doing this
	// there is a way to get the root container (this)
	public static AssetMap getApplet() {
		return applet;
	}

	/**
	 * Opens the specfied url in the sq_main frame of the matrix system.
	 * @param url the url to open in the sq_main frame
	 */
	public static void getURL(String url) throws MalformedURLException {
		applet.getAppletContext().showDocument(new URL(url), "sq_main");
	}

	/**
	 * Operns a new browser window to the specified url
	 * @param url the url to open
	 * @param title the title to show in the browser window
	 * @see getURL(String)
	 */
	public static void openWindow(String url, String title) {
		if (window == null)
			window = JSObject.getWindow(applet);
		window.call("open_hipo", new Object[] { url } );
	}

	public void initialisationComplete(InitialisationEvent evt) {}

	
	/**
	 * Initialises the asset map.
	 * @see start()
	 * @see init()
	 */
	public void init() {
		window = JSObject.getWindow(this);
		loadParameters();
		getContentPane().add(createApplet());
	}

	/**
	 * Starts the applet.
	 * This is where the initial request is made to the matrix system for
	 * the current assets and asset types. The request happens in a swing safe
	 * worker thread and the GUI is updated upon completion of the request.
	 * @see JApplet.start()
	 * @see stop()
	 * @see init()
	 */
	public void start() {
		initAssetMap();
		startPolling();
	}
	
	/**
	 * Initialises the asset map by making a request to the matrix system for
	 * the current assets and asset types in the system.
	 */
	protected void initAssetMap() {
		// get a swing worker to call init in AssetManager
		// when it returns set the root node to all trees
		MatrixStatusBar.setStatus("Initialising...");
		SwingWorker worker = new SwingWorker() {
			public Object construct() {
				MatrixTreeNode root = null;
				try {
					root = AssetManager.init();
				} catch (IOException ioe) {
					return ioe;
				}
				return root;
			}
			
			public void finished() {
				Object get = get();
				// success
				if (get instanceof MatrixTreeNode) {
					MatrixTreeModelBus.setRoot((MatrixTreeNode) get());
					MatrixStatusBar.setStatusAndClear("Success!", 1000);
				// error
				} else if (get instanceof IOException) {
					IOException ioe = (IOException) get;
					GUIUtilities.error(
						AssetMap.this, ioe.getMessage(), "Initilisation Failed!");
					MatrixStatusBar.setStatusAndClear("Initilisation Failed!", 1000);
					Log.log("Could not initialise the asset map", AssetMap.class, ioe);
				}
			}
		};
		worker.start();
	}
	
	/**
	 * Starts the polling operation to refresh assets that Matrix deems stale.
	 * The polling operation is polled at POLLING_TIME intervals. When javascript
	 * array is not empty, the assetids are used in a refresh operation.
	 * @see POLLING_TIME
	 */
	protected void startPolling() {
		// Polling timer
		ActionListener taskPerformer = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				if (window == null)
					JSObject.getWindow(AssetMap.this);

				String assetidsStr = (String) window.getMember("SQ_REFRESH_ASSETIDS");
				// if the string isn't empty, we have some work to do.
				// split the string for the asset ids and get the refresh worker
				// to start the refresh operation
				if (assetidsStr != null && !assetidsStr.equals("")) {
					String[] assetids = assetidsStr.split(",");
					AssetRefreshWorker worker = new AssetRefreshWorker(assetids, false) {
						// return a custom message for the wait message
						protected String getStatusBarWaitMessage() {
							return "Auto Refreshing...";
						}
					};
					worker.start();
					// clear the assets that we have refreshed
					window.eval("SQ_REFRESH_ASSETIDS = '';");
				}
			}
		};
		timer = new javax.swing.Timer(POLLING_TIME, taskPerformer);
		timer.start();
	}
	
	/**
	 * Load the parameters from the applet tag and add them to
	 * our property set. Parameters are stored in the Matrix property set
	 * and can be accessed with:
	 * <pre>
	 *  String prop = Matrix.getParameter("parameter." + propertyName);
	 * </pre>
	 * @see Matrix.getParameter(String)
	 */
	public void loadParameters() {
		// get the list of parameters availble to the asset map
		// and store them in the matrix property set.
		String paramStr = getParameter("parameter.params");
		String[] params = paramStr.split(",");

		for (int i = 0; i < params.length; i++)
			Matrix.setProperty(params[i], getParameter(params[i]));
	}

	/**
	 * Stops the asset map and polling.
	 * @see start()
	 * @see init()
	 */
	public void stop() {
		timer.stop();
		timer = null;
	}

	/**
	 * Creates the asset map's interface.
	 * @return the compoent to add to the content pane.
	 */
	protected JComponent createApplet() {
		getContentPane().setBackground(new Color(0x342939));

		pane = new MatrixTabbedPane(JTabbedPane.LEFT);
		view1 = new BasicView();
		view2 = new BasicView();

		pane.addView("Tree One", view1);
		pane.addView("Tree Two", view2);

		return pane;
	}

	/**
	 * Method stub for javascript calls to java.
	 * Passes on the work to the <code>JsEventManager</code> to invoke
	 * the method in the listeners.
	 *
	 * @param	type		The type of event
	 * @param	command		The event command. This is actually the
	 * name of the method that will eventually get  called in the listener.
	 * <br /><br />
	 * For Example.<br /> When javascript invokes the asset finder,
	 * a command of AssetFinderStarted is passed as this argument,
	 * which will call the method AssetFinderStarted in any listeners
	 * implementing the interface <code>AssetFinderListener</code>.
	 * @param params a serialised javascript array using the serialiseArray()
	 * function in <code>fudge/var_serialise/var_serialize.js</code>
	 */
	public void jsToJavaCall(String type, String command, String params) {
		if (type.equals("asset_finder")) {
			processAssetFinder(command, params);
		}
	}

	private void processAssetFinder(String command, String params) {
		if (command.equals("assetFinderStarted")) {
			String[] assetTypes = params.split(",");
			MatrixTreeBus.startAssetFinderMode(assetTypes);
		} else if (command.equals("assetFinderStopped")) {
			MatrixTreeBus.stopAssetFinderMode();
		}
	}
}

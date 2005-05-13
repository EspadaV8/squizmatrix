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
 * $Id: FinderTree.java,v 1.1 2005/05/13 07:06:51 ndvries Exp $
 * $Name: not supported by cvs2svn $
 */

 /*
  * :tabSize=4:indentSize=4:noTabs=false:
  * :folding=explicit:collapseFolds=1:
  */

package net.squiz.matrix.matrixtree;

import net.squiz.cuetree.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
import net.squiz.matrix.assetmap.*;

import javax.swing.tree.*;
import javax.swing.event.*;
import javax.swing.*;

import java.io.IOException;
import java.util.*;
import java.net.*;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.awt.geom.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;

/**
 * The FinderTree class is the tree used when in finder mode.
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class FinderTree extends MatrixTree {

	public FinderTree() {
		super();
	}

	public FinderTree(TreeModel model) {
		super(model);
	}

	protected MenuHandler getMenuHandler() {
		return new FinderMenuHandler();
	}

	protected DoubleClickHandler getDoubleClickHandler() {
		return null;
	}

	protected DragHandler getDragHandler() {
		return null;
	}

	protected DropHandler getDropHandler() {
		return null;
	}

	protected CueGestureHandler getCueGestureHandler() {
		return null;
	}

	protected class FinderMenuHandler extends MenuHandler {

		private ActionListener addMenuListener;

		/**
		 * Constructs menu handler
		 * @return the menu handler
		 */
		public FinderMenuHandler() {
			addMenuListener = MatrixMenus.getMatrixTreeAddMenuListener(FinderTree.this);
		}

		/**
		 * Event listener method that is called when the mouse is clicked
		 * @param evt the MouseEvent
		 */
		public void mouseClicked(MouseEvent evt) {

			if (!GUIUtilities.isRightMouseButton(evt))
				return;

			JPopupMenu menu = null;

			if (getPathForLocation(evt.getX(), evt.getY()) == null) {
				return;
			} else {
				TreePath[] selectedPaths = getSelectionPathsForLocation(evt.getX(), evt.getY());

				if (selectedPaths.length == 1) {
					setSelectionPaths(selectedPaths);
					MatrixTreeNode node = getSelectionNode();
					if (MatrixTreeBus.typeIsRestricted(node.getAsset().getType()) && isInAssetFinderMode()) {
						menu = MatrixMenus.getUseMeMenu(node);
					}
				}
			}
			if (menu != null)
				menu.show(FinderTree.this, evt.getX(), evt.getY());
		}


	}//end class MenuHandler


}//end class FinderTree

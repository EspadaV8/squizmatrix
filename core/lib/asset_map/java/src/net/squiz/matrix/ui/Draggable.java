/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: Draggable.java,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.ui;

import java.awt.*;
import javax.swing.tree.*;

public interface Draggable {
	public Image getDragImage(TreePath[] path);
}

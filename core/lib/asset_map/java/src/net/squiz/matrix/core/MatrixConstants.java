
package net.squiz.matrix.core;

import java.awt.Color;
import java.awt.Font;

public interface MatrixConstants {

	/** Asset Archival status. */
	public static final int ARCHIVED = 1;
	/** Asset Under Construction status. */
	public static final int UNDER_CONSTRUCTION = 2;
	/** Asset Workflow Pending Approval status. */
	public static final int PENDING_APPROVAL = 4;
	/** Asset Workflow Approved status. */
	public static final int APPROVED = 8;
	/** Asset Live status. */
	public static final int LIVE = 16;
	/** Asset Live Under Review status. */
	public static final int LIVE_APPROVAL = 32;
	/** Asset Safe Editing status */
	public static final int EDITING = 64;
	/** Asset Workflow Approval Safe Editing status. */
	public static final int EDITING_APPROVAL = 128;
	/** Asset Workflow Approved Safe Editing status. */
	public static final int EDITING_APPROVED = 256;

	/** The Acrhived colour */
	public static final Color ARCHIVED_COLOUR = new Color(0x655240);
	/** The Under Construction colour */
	public static final Color UNDER_CONSTRUCTION_COLOUR = new Color(0xAACCDD);
	/** The Pending Approval colour */
	public static final Color PENDING_APPROVAL_COLOUR = new Color(0xDCD2E6);
	/** The Approved colour */
	public static final Color APPROVED_COLOUR = new Color(0xF4D425);
	/** The Live colour */
	public static final Color LIVE_COLOUR = new Color(0xDBF18A);
	/** The Live Approval colour */
	public static final Color LIVE_APPROVAL_COLOUR = new Color(0x50D000);
	/** The Editing Colour */
	public static final Color EDITING_COLOUR = new Color(0xF25C86);
	/** The Editing Approval Colour */
	public static final Color EDITING_APPROVAL_COLOUR = new Color(0xCC7CC7);
	/** The Editing Approved Colour */
	public static final Color EDITING_APPROVED_COLOUR = new Color(0xFF9A00);
	/** A colour that will be used if the status colour is unknown */
	public static final Color UNKNOWN_STATUS_COLOUR = Color.white;
	
	/** plain font 10pt */
	public static final Font PLAIN_FONT_10 = new Font("plain_font_10", Font.PLAIN, 10);

}

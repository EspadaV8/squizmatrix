
package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;

public class ExpandingNextNode extends ExpandingNode {

	public ExpandingNextNode(int parentTotalAssets, int currentAssetsCount, int viewedAssetsCount) {
		setParentTotalAssets(parentTotalAssets);
		setCurrentAssetsCount(currentAssetsCount);
		setViewedAssetsCount(viewedAssetsCount);
		Asset expandingAsset = new ExpandingAsset(getName());
		setUserObject(expandingAsset);
	}

	public int getStartLoc(int evtX, double boundsX) {
		int res = evtX - (int)boundsX;
		if (res >=0 && res <=12) {
			// first img clicked, we will get next set of nodes
			return getViewedAssetsCount() + AssetManager.getLimit();
		} else if (res >=13 && res <=35) {
			// second img clicked, get the last set of nodes
			if (getParentTotalAssets() < 0) {
				return -1;
			}
			int loc = getParentTotalAssets() - (getParentTotalAssets()%AssetManager.getLimit());
			if (loc == getParentTotalAssets())
				loc -= AssetManager.getLimit();
			return loc;
		} else {
			return -1;
		}
	}

}

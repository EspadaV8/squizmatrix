<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: locking_method_default.inc,v 1.1 2009/09/21 00:27:45 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/locking/locking_method/locking_method.inc';


/**
* Locking_Method_Default
*
* Purpose
*    System locking mechanism using the default method of database storage in
*    the sq_cache table.
*
*    When using databases in this locking method, you should make sure you are
*    in the "db3" connection. If database connections are set up properly, this
*    means that locking will be able to bypass current transactions on db2 and
*    ensure locks are made as quickly as possible.
*
*
* @author		Luke Wright <lwright@squiz.net>
* @version		$Revision: 1.1 $
* @package		MySource_Matrix_Packages
* @subpackage	__core__
*/
class Locking_Method_Default extends Locking_Method
{


	/**
	* Constructor
	*
	* This doesn't really matter in this case, as it will only be called
	* statically.
	*
	* @param string	$assetid	The ID of the asset to load
	*
	*/
	public function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end __construct()


	/**
	* Acquire a lock on part of the system
	*
	* The $source_lockid parameter should be set if an asset is being locked
	* as a result of a dependant parent being locked. If the source lock is
	* released, this lock will be too. If this lock is not dependent on any
	* other, use blank.
	*
	* Setting $expires to zero (or omitted) will result in the default lock
	* length being used (as defined as SQ_CONF_LOCK_LENGTH in the System
	* Configuration file).
	*
	* If $expires is NULL, there will be no expiry time set on the lock. It will
	* remain locked until unlocked by the user.
	*
	* Otherwise, $expires should be a UNIX timestamp of the expiry time. (ie. it
	* is not a lock length.)
	*
	* The locking method should throw an exception if there is a problem.
	*
	* @param string	$lockid			The ID of the lock to acquire
	* @param string	$source_lockid	The ID that triggered the lock acquisition
	* @param int	$expires		Timestamp of the expiry time
	*
	* @return boolean
	*/
	public static function acquireLock($lockid, $source_lockid='', $expires=0)
	{
		$current_lock = $GLOBALS['SQ_SYSTEM']->getLockInfo($lockid);

		// is this asset already locked
		if (!empty($current_lock)) {
			// the user is asking to acquire a lock they already had
			// so just update the lock expiry date
			if ($current_lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
				return self::updateLock($lockid, $expires);
			} else {
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);
				throw new Exception('Lock already held by "'.$user->name.'"');
			}
		}

		// if we have no source, we are the asset aquiring the locks
		if (empty($source_lockid)) $source_lockid = $lockid;

		if (!is_null($expires)) {
			$expires = (empty($expires)) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;
			$expires = ts_iso8601($expires);
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'INSERT INTO sq_lock
				(lockid, source_lockid, userid, expires)
				VALUES
				(:lockid, :source_lockid, :userid, :expires)';

		try {
			if (MatrixDAL::getDbType() == 'oci') {
				$sql = str_replace(':expires', db_extras_todate(MatrixDAL::getDbType(), ':expires', FALSE), $sql);
			}

			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'lockid',        $lockid);
			MatrixDAL::bindValueToPdo($query, 'source_lockid', $source_lockid);
			MatrixDAL::bindValueToPdo($query, 'userid',        $GLOBALS['SQ_SYSTEM']->currentUserId());
			MatrixDAL::bindValueToPdo($query, 'expires',       $expires);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			// Rather than throwing our own exception, keep the previous format
			// of returning an error message for now
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			throw new Exception('DB error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end acquireLock()


	/**
	* Update the expiry time on a lock
	*
	* The expiry time will be updated on all locks that share the same source
	* lock ID as $lockid. The $expires parameter follows the same rules as
	* acquireLock().
	*
	* If a lock is set to indefinite (NULL), this function will not change this,
	* but will still return TRUE. You must release an indefinite lock before
	* setting a definite expiry date on it.
	*
	* The locking method should throw an exception if there is a problem with
	* updating the lock expiry.
	*
	* @param string	$lockid			The ID of the lock to update
	* @param int	$expires		Timestamp of the expiry time
	*
	* @see Locking_Method::acquireLock()
	* @return boolean
	*/
	public static function updateLock($lockid, $expires=0)
	{
		$current_lock = self::getLockInfo($lockid);

		// If there is no current lock (it may have expired), or if the lock
		// is indefinite, do nothing
		if (empty($current_lock) || empty($current_lock['expires'])) {
			return TRUE;
		}

		if (!is_null($expires)) {
			$expires = (!$expires) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;
			$expires = ts_iso8601($expires);
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'UPDATE sq_lock
				   SET expires = :expires
				   WHERE source_lockid = :source_lockid';

		try {
			$sql = str_replace(':expires', db_extras_todate(MatrixDAL::getDbType(), ':expires', FALSE), $sql);

			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'expires',       $expires);
			MatrixDAL::bindValueToPdo($query, 'source_lockid', $current_lock['source_lockid']);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			// Rollback and then re-throw, let parent call handle the exception
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			throw new Exception('DB error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end acquireLock()


	/**
	* Release a lock
	*
	* All locks that share the same source lock ID as $lockid will be released,
	* to ensure that dependant groups of assets (for which the source lock is
	* primarily used for) are locked and unlocked in one group.
	*
	* @param string	$lockid			The ID of the lock to release
	*
	* @see Locking_Method::acquireLock()
	* @return boolean
	*/
	public static function releaseLock($lockid)
	{
		$current_lock = self::getLockInfo($lockid, FALSE, FALSE);
		if (empty($current_lock)) return TRUE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM sq_lock
			WHERE source_lockid = :source_lockid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'source_lockid', $current_lock['source_lockid']);
			$result = MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			// Rather than throwing our own exception, keep the previous format
			// of returning an error message for now
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			throw new Exception('DB error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end releaseLock()


	/**
	* Returns an array of information about the passed lock
	*
	* If $full_chain is TRUE, all locks with the same source lock ID as that
	* of $lockid will be returned (this could return locks from both parents and
	* children if this is in the middle of a full chain).
	*
	* If $check_expires is TRUE, any locks that have expired will be released,
	* and not returned. Only locks that would have been returned by this
	* function are tested; it does not do a general "sweep" of the locks table.
	*
	* @param string|array	$lockid			The id of the lock to get info for
	* @param boolean		$full_chain		TRUE = Return the full lock chain
	* @param boolean		$check_expires	TRUE = Clean up expired locks found
	* @param boolean		$allow_only_one	TRUE = Force only one lock returned
	*
	* @return array
	* @access public
	*/
	public static function getLockInfo($lockid, $full_chain=FALSE, $check_expires=TRUE, $allow_only_one=TRUE)
	{
		// Change to the "locks" connection
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');

		// Did I get a string or an array?
		$where = '';
		$multiple = FALSE;
		$locks_passed = Array();
		if (is_array($lockid)) {
			foreach ($lockid as $lock_key => $lock_value) {
				$lockid[$lock_key] = MatrixDAL::quote($lock_value);
			}
			$where = ' WHERE lockid IN ('.implode(', ', $lockid).')';
			$multiple = TRUE;
			$locks_passed = $lockid;
		} else {
			$where = ' WHERE lockid = :lockid';
			$multiple = FALSE;
			$locks_passed[] = $lockid;
		}

		$sql = 'SELECT lockid, source_lockid, userid, expires
				FROM sq_lock'.$where;

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			if (!$multiple) {
				MatrixDAL::bindValueToPdo($query, 'lockid', $lockid);
			}
			$results = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			$lock_error = (is_array($lockid)) ? ('s '.implode(', ', $lockid)) : (' '.$lockid);
			throw new Exception('Unable to get lock information for lock ID"'.$lock_error.'" due to database error: '.$e->getMessage());
		}

		if (!empty($results)) {
			foreach ($results as $result_key => $result) {
				$results[$result_key]['expires'] = empty($result['expires']) ? NULL : iso8601_ts($result['expires']);
				if ($check_expires && !empty($result['expires'])) {
					if ($results[$result_key]['expires'] < time()) {
						self::releaseLock($lockid);
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						// this lock has expired
						return Array();
					}
				}

				if ($full_chain) {
					$results[$result_key]['chained_assets'] = Array();
					foreach ($locks_passed as $lock_to_test) {
						$sql = 'SELECT lockid, source_lockid, userid, expires
								FROM sq_lock
								WHERE source_lockid = :source_lockid
									AND lockid <> :lockid';

						try {
							$query = MatrixDAL::preparePdoQuery($sql);
							MatrixDAL::bindValueToPdo($query, 'source_lockid', $result['source_lockid']);
							MatrixDAL::bindValueToPdo($query, 'lockid', $lock_to_test);
							$chain = MatrixDAL::executePdoAssoc($query);
						} catch (Exception $e) {
							throw new Exception('Unable to get lock chain information for source lock ID "'.$result['source_lockid'].'" due to database error: '.$e->getMessage());
						}

						foreach ($chain as $lock_row) {
							$lock_row['expires'] = (empty($lock_row['expires'])) ? NULL : iso8601_ts($lock_row['expires']);
							$results[$result_key]['chained_assets'][$lock_row['lockid']] = $lock_row;
						}
					}//end foreach
				}//end if
			}//end foreach
		}//end if
		if (!empty($results) && $allow_only_one) {
				$results = $results[0];
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $results;

	}//end getLockInfo()


	/**
	* Get a list of all active locks, optionally filtered by lock owner
	*
	* Requires the ability to filter the list of locks by owner asset ID.
	*
	* If $owner_assetid omitted, all locks will be returned.
	*
	* @param string		$owner_assetid		The asset ID of the owning user to filter
	* @param boolean	$include_expired	TRUE = expired locks will also be returned
	*
	* @return array
	*/
	public static function getActiveLocks($owner_assetid=NULL, $include_expired=FALSE)
	{
		try {
			$bind_vars = Array();

			if ($include_expired === FALSE) {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$date = ts_iso8601(time());
				$time_val = MatrixDAL::executeOne('core', 'toDate', Array('date_value' => $date));
				$bind_vars['expire_time'] = $time_val;
			} else {
				$bind_vars['expire_time'] = NULL;
			}

			if ($owner_assetid !== NULL) {
				$bind_vars['owner_assetid'] = $owner_assetid;
			} else {
				$bind_vars['owner_assetid'] = NULL;
			}

			$db_locks = MatrixDAL::executeAll('core', 'getActiveLocks', $bind_vars);

		} catch (Exception $e) {
			throw $e;

		}//end try catch

		return $db_locks;

	}//end getActiveLocks()


	/**
	* Does this locking method support the getting of active locks?
	*
	* @return boolean
	*/
	public static function supportsGetActiveLocks()
	{
		return TRUE;

	}//end supportsGetActiveLocks()


	/**
	* Change the owner of a lock from one user to another
	*
	* Requires the ability to filter the list of locks by owner asset ID.
	*
	* Not entirely useful with core users. Mostly useful for users being drawn
	* in from external sources, such as LDAP, where the DN might change and
	* therefore, possibly the user.
	*
	* @param string	$old_assetid	The asset ID of the old user owner
	* @param string	$new_assetid	The new user asset ID to own the locks
	*
	* @return boolean
	*/
	public static function changeLockOwner($old_assetid, $new_assetid)
	{
		$bind_vars = Array(
					'old_userid'	=> $old_assetid,
					'new_userid'	=> $new_assetid,
				 );
		MatrixDAL::executeQuery('core', 'changeAllLocksHeldUser', $bind_vars);
		return TRUE;

	}//end changeLockOwner()


	/**
	* Does this locking method support the changing of lock ownership?
	*
	* @return boolean
	*/
	public static function supportsChangeLockOwner()
	{
		return TRUE;

	}//end supportsChangeLockOwner()


}//end class

?>
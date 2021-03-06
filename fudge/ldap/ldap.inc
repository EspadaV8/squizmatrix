<?php
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
*/


// Page size of LDAP search result when using pagination control
define('SQ_LDAP_RESULT_PAGE_SIZE', 500);


/**
* LDAP
*
* Purpose
*     To provide an interface to an LDAP directory server for basic functionality
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.17 $
* @package Fudge
* @subpackage ldap
*/
class Ldap
{

	/**
	* The host used to connect to the LDAP server
	* Abreviated to 'h' throughout the member functions
	*
	* @var string
	*/
	var $host = '';

	/**
	* The port used to connect to the LDAP server
	* Abreviated to 'p' throughout the member functions
	*
	* @var int
	*/
	var $port = 0;

	/**
	* The current LDAP server pointer
	*
	* @var resource
	*/
	var $ptr = null;


	/**
	* Constructor
	*
	* @param string	$h		the host used to connect to the LDAP server
	* @param int	$p		the port used to connect to the LDAP server
	* @param string	$pass	the password used to connect to the LDAP server
	* @param string	$bdn	the DN used to bind to the LDAP server
	*
	*/
	function Ldap($h=null, $p=null, $pass=null, $bdn='')
	{
		// if a host has been specified, connect
		if (!is_null($h) && !empty($h)) {
			$this->connect($h, $p, $pass, $bdn);
			$this->bind($bdn, $pass);
		}

	}//end constructor


	/**
	* Connect to a certain LDAP server and bind to it
	*
	* @param string		$h	the host used to connect to the LDAP server
	* @param int		$p	the port used to connect to the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function connect($h, $p=null)
	{
		// bail out safely if we have no ldap functions
		if (!extension_loaded('ldap')) {
			$this->ptr = null;
			return false;
		}

		if ($h != $this->host || $p != $this->port) {
			// close old connection
			if ($this->ptr) ldap_close($this->ptr);

			if (!($this->ptr = ldap_connect($h, $p))) {
				trigger_error('Unable to connect to LDAP server: '.$h.' on port '.$p, E_USER_WARNING);
				return false;
			}
			$this->host = $h;
			$this->port = $p;
		}
		return true;

	}//end connect()


	/**
	* Bind to a LDAP server
	*
	* @param string		$bdn	the DN used to bind to the LDAP server
	* @param string		$pass	the password used to connect to the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function bind($bdn='', $pass=null)
	{
		if (!$this->ptr) return false;

		// bind to the server
		if (is_null($pass)) {
			// attempting an anonymous bind
			if (!($r = ldap_bind($this->ptr))) {
				trigger_error('Unable to Anonomously bind to LDAP server: '.$this->host, E_USER_WARNING);
				unset($this->ptr);
				return false;
			}
		} else {
			if (!($r = ldap_bind($this->ptr, $bdn, $pass))) {
				trigger_error('Unable to bind to LDAP server: '.$this->host.' BDN: '.$bdn.' Using Password: '.(($pass) ? 'YES' : 'NO'), E_USER_WARNING);
				$this->ptr = null;
				return false;
			}
		}

		return $r;

	}//end bind()


	/**
	* Disconnect from the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function disconnect()
	{
		if ($this->ptr) {
			return ldap_close($this->ptr);
		}
		return true;

	}//end disconnect()


	/**
	* Connect to a certain LDAP server and bind to it as a particular user
	*
	* @param string		$bdn	the root DN used to connect to the LDAP server
	* @param string		$pass	the password used to connect to the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function connectAsUser($bdn, $pass)
	{
		$conn_result = $this->connect($this->host, $this->port);
		if (!$conn_result) return false;
		return $this->bind($bdn, $pass);

	}//end connectAsUser()


	/**
	* Search the server starting at $startdn using $filter
	*
	* NOTE: This function usage in Matrix package has been replaced by searchEntries()
	* which will return the actual ldap entries instead of LDAP search resource
	* This function is not removed or renamed for backward compatibilty (custom client package, if any, using this class)
	*
	* @param string		$startdn		the DN to start searching at
	* @param string		$filter			search filter
	* @param boolean	$multi_level	search multiple levels
	* @param boolean	$multi_result	search for multiple entries
	* @param array		$attributes		array of the attributes we want to return for the returned objects
	*
	* @return resource (search result identifier) | zero (on error)
	* @access public
	*/
	function search($startdn, $filter, $sort_by=null, $multi_level=true, $multi_result=true, $attributes=Array())
	{
		if (!$this->ptr) return 0;

		if(!empty($attributes)) {
			if ($multi_level && $multi_result) $sr = @ldap_search($this->ptr, $startdn, $filter, $attributes);
			else if ($multi_result) $sr = @ldap_list($this->ptr, $startdn, $filter, $attributes);
			else $sr = @ldap_read($this->ptr, $startdn, $filter, $attributes);
		} else {
			if ($multi_level && $multi_result) $sr = @ldap_search($this->ptr, $startdn, $filter);
			else if ($multi_result) $sr = @ldap_list($this->ptr, $startdn, $filter);
			else $sr = @ldap_read($this->ptr, $startdn, $filter);
		}

		if (!$sr) {
			$ldap_error = ldap_err2str(ldap_errno($this->ptr));
			log_error($ldap_error.' Filter string: '.$filter);
			return 0;
		} else {
			if (!is_null($sort_by)) ldap_sort($this->ptr, $sr,  $sort_by);
			return $sr;
		}

	}//end search()


	/**
	* Search the server starting at $startdn using $filter
	* and return the LDAP entries
	*
	* @param string		$startdn		the DN to start searching at
	* @param string		$filter			search filter
	* @param string		$sort_by		attribute to sort by
	* @param boolean	$multi_level	search multiple levels
	* @param boolean	$multi_result	search for multiple entries
	* @param array		$attributes		array of the attributes we want to return for the returned objects
	* @param array		$binary_attributes 	the attributes that need to be extracted under binary format
	* @param string		$member_attribute 	the group name that defines its member
	*
	* @return array|boolean
	* @access public
	*/
	function searchEntries($startdn, $filter, $sort_by=null, $multi_level=true, $multi_result=true, $attributes=Array(), $binary_attributes = Array(), $member_attribute = '')
	{
		if (!$this->ptr) {
			return FALSE;
		}

		if (empty($attributes)) {
			$attributes = Array();
		}

		if (!$this->_paginateSearch()) {
			if ($multi_level && $multi_result) $sr = @ldap_search($this->ptr, $startdn, $filter, $attributes);
			else if ($multi_result) $sr = @ldap_list($this->ptr, $startdn, $filter, $attributes);
			else $sr = @ldap_read($this->ptr, $startdn, $filter, $attributes);

			if (!$sr) {
				$ldap_error = ldap_err2str(ldap_errno($this->ptr));
				log_error($ldap_error.' Filter string: '.$filter);
				return FALSE;
			}

			$entries = $this->getEntries($sr, $binary_attributes);
		} else {

			$entries = Array('count' => 0);
			$cookie = '';
			do {
				// Set the result page size
				ldap_control_paged_result($this->ptr, SQ_LDAP_RESULT_PAGE_SIZE, FALSE, $cookie);

				if ($multi_level && $multi_result) $sr = @ldap_search($this->ptr, $startdn, $filter, $attributes);
				else if ($multi_result) $sr = @ldap_list($this->ptr, $startdn, $filter, $attributes);
				else $sr = @ldap_read($this->ptr, $startdn, $filter, $attributes);

				if (!$sr) {
					$ldap_error = ldap_err2str(ldap_errno($this->ptr));
					log_error($ldap_error.' Filter string: '.$filter);
					return FALSE;
				}
				$page_entries = $this->getEntries($sr, $binary_attributes);
				if (!isset($page_entries['count'])) {
					break;
				}

				$page_entries['count'] += $entries['count'];
				$entries = array_merge($entries, $page_entries);

				if (!@ldap_control_paged_result_response($this->ptr, $sr, $cookie)) {
					break;
				}

			} while ($cookie !== NULL && $cookie != '');
		}

		// Handle paginated member list in member attribute value
		if (!empty($member_attribute)) {
			foreach($entries as &$entry) {
				if ($entry == 'count' || empty($entry['dn'])) {
					continue;
				}
				$paginated_list_attr = NULL;
				foreach($entry as $attr_name => $attr_val) {
					// If the member list in member attribute is paginated
					if (preg_match('/^'.preg_quote($member_attribute).';range=\d+\-\d+$/', $attr_name)) {
						$paginated_list_attr = $attr_name;
					}
				}//end foreach

				// The member list in member attribute is paginated, we need to get the complete list
				if ($paginated_list_attr && isset($entry[$paginated_list_attr]) && isset($entry[$paginated_list_attr]['count'])) {
					$members = $entry[$paginated_list_attr];
					$page_size = $entry[$paginated_list_attr]['count'];
					unset($entry[$paginated_list_attr]);
					$start_range = $page_size;
					while(TRUE) {
						// Get the group entry with the specified members range
						$member_attr = $member_attribute.';range='.$start_range.'-'.($start_range + $page_size - 1);
						$group_sr = @ldap_read($this->ptr, $entry['dn'], '(objectClass=*)', Array($member_attr));
						if (!$group_sr) {
							$ldap_error = ldap_err2str(ldap_errno($this->ptr));
							log_error($ldap_error.' Filter string: '.$filter);
							return FALSE;
						}

						// This should return single entry only with updated paginated member list attribute
						$group_entries = $this->getEntries($group_sr);
					
						// If empty result then we have reach the end of the member list
						if (!isset($group_entries[0]) || !isset($group_entries[0]['count']) || $group_entries[0]['count'] != 1 || !isset($group_entries[0][$member_attr]) || !isset($group_entries[0][$member_attr]['count']) || $group_entries[0][$member_attr]['count'] == 0) {
							break;
						}

						//Add up the members into single list
						$members = array_merge($members, $group_entries[0][$member_attr]);
						$start_range += $page_size;
					}//end while

					if (!empty($members)) {
						unset($members['count']);
						$members['count'] = count($members);
						// Set the 'member' attribute to the complete member list
						$entry[$member_attribute] = $members;
					}
				}//end if
			}//end foreach
		}//end if member attribute

		// Sort the result if required
		if ($sort_by) {
			uasort($entries, function($a, $b) use ($sort_by) {
				$a = isset($a[$sort_by][0]) ? $a[$sort_by][0] : '';
				$b = isset($b[$sort_by][0]) ? $b[$sort_by][0] : '';

				return ($a == $b) ? 0 : (($a < $b) ? -1 : 1);
			});
		}

		if (!isset($entries['count'])) {
			$entries = Array('count' => 0);
		}

		return $entries;

	}//end searchEntries()


	/**
	* Get the entries in a given search result in a multi-dim array
	*
	* @param resource	$result	the search result identifier
	* @param array		$binary_attributes the attributes that need to be extracted under binary format
	*
	* @return array
	* @access public
	*/
	function getEntries($result, $binary_attributes = Array())
	{
		if (!($info = ldap_get_entries($this->ptr, $result))) {
			trigger_error('Unable to get entries for LDAP search result ['.$result.'] :'.ldap_error($this->ptr), E_USER_WARNING);
			return Array();
		} else {
			// ldap_get_entries() function can not handle binary data so we use ldap_get_values_len() to
			// put binary data to the binary attributes of $info
			if (!empty($binary_attributes) && ($info['count'] > 0)) {
				// There is no document confirming that the order of entries returned by ldap_get_entries()
				// is the same with that of ldap_first_entry()/ldap_next_entry() so we use the dn as identifier
				$entry_order = Array();
				for ($i = 0; $i < $info['count']; $i++) {
					$entry_order[$info[$i]['dn']] = $i;
				}
				// Set binary data for each entry
				for ($entry_identifier = ldap_first_entry($this->ptr, $result); $entry_identifier !== FALSE; $entry_identifier = ldap_next_entry($this->ptr, $entry_identifier)) {
					$dn = ldap_get_dn($this->ptr, $entry_identifier);
					if (($dn !== FALSE) && isset($entry_order[$dn])) {
						$entry_index = $entry_order[$dn];
						foreach ($binary_attributes as $attribute) {
							// ldap_get_entries() lowercases attribute indexes
							$lowercased_attribute = strtolower($attribute);
							// ldap_get_entries() can still get the first few bytes of the binary data, so we can speed this function
							// a bit by checking if there is a value of this attribute before extracting its actual binary data
							if (isset($info[$entry_index][$lowercased_attribute])) {
								// ldap_get_values_len() function throws a warning if there is no value for the attribute so we suppress the warning with @
								$binary_value = @ldap_get_values_len($this->ptr, $entry_identifier, $attribute);
								if ($binary_value !== FALSE) {
									$info[$entry_index][$lowercased_attribute] = $binary_value;
								}
							}
						}
					}
				}
			}

			return $info;
		}

	}//end getEntries()


	/**
	* Return the number of entries in a given search result
	*
	* @param resource	$result	the search result identifier
	*
	* @return int
	* @access public
	*/
	function getNumEntries($result)
	{
		if (!$result) return 0;
		return ldap_count_entries($this->ptr, $result);

	}//end getNumEntries()


	/**
	* Insert an entry into LDAP
	*
	* @param string	$dn			the level to add the entry at
	* @param array	$details	the details for new entry
	*
	* @return boolean
	* @access public
	*/
	function addEntry($dn, $details)
	{
		if (!ldap_add($this->ptr, $dn, $details)) {
			trigger_error('Unable to INSERT entry into LDAP at DN: ['.$dn.'] '.ldap_error($this->ptr), E_USER_WARNING);
			return false;
		}
		return true;

	}//end addEntry()


	/**
	* Modify an LDAP entry
	*
	* @param string	$dn			the DN of the entry to modify
	* @param array	$details	the new details for entry
	*
	* @return boolean
	* @access public
	*/
	function modifyEntry($dn, $details)
	{
		if (!ldap_modify($this->ptr, $dn, $details)) {
			trigger_error('Unable to MODIFY entry in LDAP at DN: ['.$dn.'] '.ldap_error($this->ptr), E_USER_WARNING);
		}
		return true;

	}//end modifyEntry()


	/**
	* Delete an entry from LDAP
	*
	* @param string	$dn		the DN of the entry to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteEntry($dn)
	{
		if (!ldap_delete($this->ptr, $dn)) {
			trigger_error('Unable to DELETE entry from LDAP at DN: ['.$dn.'] '.ldap_error($this->ptr), E_USER_WARNING);
		}
		return true;

	}//end deleteEntry()


	/** 
	* Escapes the given string according to RFC 2254 so that they can be safely used in LDAP filters.
	* 
	* @param string	$str	the string to escape
	* 
	* @return string
	* @access public
	*/
	function escape_filter_value($str){
		
	    $metaChars = array('\\', '(', ')', '*');
	    $quotedMetaChars = array();
	    foreach ($metaChars as $key => $value) $quotedMetaChars[$key] = '\\'.dechex(ord($value));
	    $str=str_replace($metaChars,$quotedMetaChars,$str); //replace them
	    return ($str);
	    
	}//end escape_filter_value()


	/**
	* Whether to use LDAP pagination control whne doing LDAP search
	*
	* @return boolean
	* @access private
	*/
	function _paginateSearch()
	{
		// LDAP pagination control functionality is available from PHP 5.4 only
		return version_compare(PHP_VERSION, '5.4.0', '>=') && ldap_get_option($this->ptr, LDAP_OPT_PROTOCOL_VERSION, $ver) && $ver == '3';

	}//end _paginateSearch()


}//end class

?>

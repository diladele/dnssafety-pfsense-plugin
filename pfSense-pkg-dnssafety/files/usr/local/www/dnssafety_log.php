<?php
/*
 * dnssafety_log.php
 *
 * Copyright (c) 2019 Diladele B.V.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
require_once("guiconfig.inc");

/* Requests */
if ($_POST) {
	
	global $filter, $program;
	// Actions
	$filter  = preg_replace('/(@|!|>|<)/', "", htmlspecialchars($_POST['strfilter']));
	$program = strtolower($_POST['program']);
	
	switch ($program) {
		
		case 'access_log';
			
			$log   = "/opt/dnssafety/var/log/access.log";
			$lines = fetch_log($log);
			foreach ($lines as $line) {
				
				echo "{$line}\n";
			}
			break;
		
		case 'error_log';
			
			$log   = "/opt/dnssafety/var/log/dsdnsd.log";
			$lines = fetch_log($log);
			foreach ($lines as $line) {
				
				echo "{$line}\n";
			}
			break;
		}
}

//
//
//
function fetch_log($log) {

	global $filter, $program;
	$log = escapeshellarg($log);
	
	// Get data from form post
	$lines = escapeshellarg(is_numeric($_POST['maxlines']) ? $_POST['maxlines'] : 50);
	if (preg_match("/!/", htmlspecialchars($_POST['strfilter']))) {
		$grep_arg = "-iv";
	} else {
		$grep_arg = "-i";
	}

	// Get logs based in filter expression
	if ($filter != "") {
		exec("/usr/bin/tail -n 2000 {$log} | /usr/bin/grep {$grep_arg} " . escapeshellarg($filter). " | /usr/bin/tail -n {$lines} ", $logarr);
	} else {
		exec("/usr/bin/tail -n {$lines} {$log} ", $logarr);
	}
	// Return logs
	return $logarr;
};

?>

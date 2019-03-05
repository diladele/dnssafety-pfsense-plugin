<?php
/*
 * dnssafety.priv.inc
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
global $priv_list;

$priv_list['page-services-dnssafety'] = array();
$priv_list['page-services-dnssafety']['name'] = "WebCfg - Services: DNS Safety package";
$priv_list['page-services-dnssafety']['descr'] = "Allow access to DNS Safety package GUI";

$priv_list['page-services-dnssafety']['match'] = array();
$priv_list['page-services-dnssafety']['match'][] = "pkg.php?xml=dnssafety.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg.php?xml=dnssafety_cache.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg.php?xml=dnssafety_forwarders.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg.php?xml=dnssafety_filter.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg.php?xml=dnssafety_recategorized.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg.php?xml=dnssafety_sync.xml*";

$priv_list['page-services-dnssafety']['match'][] = "pkg_edit.php?xml=dnssafety.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg_edit.php?xml=dnssafety_cache.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg_edit.php?xml=dnssafety_forwarders.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg_edit.php?xml=dnssafety_filter.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg_edit.php?xml=dnssafety_recategorized.xml*";
$priv_list['page-services-dnssafety']['match'][] = "pkg_edit.php?xml=dnssafety_sync.xml*";

$priv_list['page-services-dnssafety']['match'][] = "shortcuts/pkg_dnssafety.inc*";
$priv_list['page-services-dnssafety']['match'][] = "dnssafety_access.php*";
$priv_list['page-services-dnssafety']['match'][] = "dnssafety_dsdnsd.php*";

?>

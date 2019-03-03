<?php
/*
 * dnssafety.inc
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
$shortcut_section = "bind";
require_once("globals.inc");
require_once("config.inc");
require_once("util.inc");
require_once("pfsense-utils.inc");
require_once("pkg-utils.inc");
require_once("service-utils.inc");
if (!function_exists("filter_configure")) {
    require_once("filter.inc");
}
require_once("notices.inc");
require_once("xmlrpc_client.inc");

//
//
//
function bind_zone_validate($post, &$input_errors) {

    if (array_key_exists("mail", $_POST)) {
        $_POST['mail'] = preg_replace("/@/", ".", $post['mail']);
    }

    if (isset($_POST['rpz']) && ($_POST['rpz'] == "on")) {
        if (($_POST['type'] !== "master") && ($_POST['type'] !== "slave")) {
            $input_errors[] = "Only a master or slave zone can be used in a response policy";
        }
    }
    switch ($_POST['type']) {
        case 'redirect':
            $_POST['tll'] = 300;
            $_POST['refresh'] = 0;
            $_POST['serial'] = 0;
            $_POST['retry'] = 0;
            $_POST['expire'] = 0;
            $_POST['minimum'] = 0;
            if ($_POST['mail'] == '') {
                $input_errors[] = "The field 'Mail Admin Zone' is required for {$_POST['type']} zones.";
            }
            // Intentionally fall through to 'master' zone case
        case 'master':
            if ($_POST['nameserver'] == '') {
                $input_errors[] = "The field 'Name server' is required for {$_POST['type']} zones.";
            }
            for ($i = 0; $i < count($_POST); $i++) {
                if (array_key_exists("hostname$i", $_POST)) {
                    if ($_POST['reverso'] == "on") {
                        $_POST["hostvalue$i"] = "";
                        if (!preg_match("/(PTR|NS)/", $_POST["hosttype$i"])) {
                            $input_errors[] = 'On reverse zones, valid record types are NS or PTR';
                        }
                    }
                    if (!preg_match("/(MX|NS)/", $_POST["hosttype$i"]) && ($_POST["hostname$i"] == "")) {
                        $input_errors[] = 'Record cannot be empty for '.$_POST["hosttype$i"].' type ';
                    }
                    if ($_POST["hosttype$i"] == "MX" && ($_POST["hostvalue$i"] == "")) {
                        $_POST["hostvalue$i"] = "10";
                    }
                    if ($_POST["hosttype$i"] != "MX" && ($_POST["hostvalue$i"] != "")) {
                        $_POST["hostvalue$i"] = "";
                    }
                    if ($_POST["hostdst$i"] == "") {
                        $input_errors[] = 'Alias or IP address cannot be empty.';
                    }
                }
            }
            break;
        case 'slave':
            if ($_POST['slaveip'] == "") {
                $input_errors[] = 'The field \'Master Zone IP\' is required for slave zones.';
            }
            break;
        case 'forward':
            if ($_POST['forwarders'] == "") {
                $input_errors[] = 'The field \'Forwarders\' is required for forward zones.';
            }
            break;
        default:
            break;
    }
}

//
//
//
function dnssafety_conf_generate_blocker($conf) {

    # set action
    $action = $conf['action'];
    if (true) {

        # ensure value is ok
        switch ($action) {
            case "redirect" : break;
            default:
                $action = "nxdomain";
        }

        # and recode
        $action = json_encode($action);
    }

    # set redirect v4
    $ipv4 = $conf['redirect_ipv4'];
    if (true) {

        # ensure value is ok
        if ($ipv4 == '') {
            $ipv4 = "127.0.0.1";
        }

        # and recode
        $ipv4 = json_encode($ipv4);
    }

    # set redirect v6
    $ipv6 = $conf['redirect_ipv6'];
    if (true) {

        # ensure value is ok
        if ($ipv6 == '') {
            $ipv6 = "::1";
        }

        # and recode
        $ipv6 = json_encode($ipv6);
    }

    # generate data
    $data = "{
        \"action\"       : $action,
        \"redirect_ip4\" : $ipv4,
        \"redirect_ip6\" : $ipv6
    }";

    # and return it
    return $data;
}

//
//
//
function dnssafety_conf_generate_logging($conf) {

    # set level
    $level = $conf['log_level'];

    # ensure value is ok
    switch ($level) {
        case "DEBUG"   : break;
        case "WARNING" : break;
        case "ERROR"   : break;
        default:
            $level = "INFO";
    }

    $tmp = json_encode($level);

    # generate data
    $data = "{
        \"level\" : $tmp
    }";

    # and return it
    return $data;
}

//
//
//
function dnssafety_conf_generate_forwarders($conf){

    # fill open dns
    $opendns = array();
    if (true) {

        # assign enable
        $opendns["enable"] = false;
        if ($conf['opendns_enable'] == "on") {
            $opendns["enable"] = true;
        }

        # assign forward fqdn only
        $opendns["forward-fqdn-only"] = false;
        if ($conf['opendns_forward_fqdn_only'] == "on") {
            $opendns["forward-fqdn-only"] = true;
        }

        # assign ipv4
        $opendns["ipv4"] = explode(",", $conf['opendns_v4']);
        $opendns["ipv4"] = array_map('trim', $opendns["ipv4"]);

        # assign ipv6
        $opendns["ipv6"] = explode(",", $conf['opendns_v6']);
        $opendns["ipv6"] = array_map('trim', $opendns["ipv6"]);

        # and encoude
        $opendns = json_encode($opendns, JSON_PRETTY_PRINT);
    };

    # fill google dns
    $google = array();
    if (true) {

        # assign enable
        $google["enable"] = false;
        if ($conf['googledns_enable'] == "on") {
            $google["enable"] = true;
        }

        # assign forward fqdn only
        $google["forward-fqdn-only"] = false;
        if ($conf['googledns_forward_fqdn_only'] == "on") {
            $google["forward-fqdn-only"] = true;
        }

        # assign ipv4
        $google["ipv4"] = explode(",", $conf['googledns_v4']);
        $google["ipv4"] = array_map('trim', $google["ipv4"]);

        # assign ipv6
        $google["ipv6"] = explode(",", $conf['googledns_v6']);
        $google["ipv6"] = array_map('trim', $google["ipv6"]);

        # and encoude
        $google = json_encode($google, JSON_PRETTY_PRINT);
    };

    # fill custom1
    $custom1 = array();
    if (true) {

        # assign enable
        $custom1["enable"] = false;
        if ($conf['customdns1_enable'] == "on") {
            $custom1["enable"] = true;
        }

        # assign forward fqdn only
        $custom1["forward-fqdn-only"] = false;
        if ($conf['customdns1_forward_fqdn_only'] == "on") {
            $custom1["forward-fqdn-only"] = true;
        }

        # assign ipv4
        $custom1["ipv4"] = explode(",", $conf['customdns1_v4']);
        $custom1["ipv4"] = array_map('trim', $custom1["ipv4"]);

        # assign ipv6
        $custom1["ipv6"] = explode(",", $conf['customdns1_v6']);
        $custom1["ipv6"] = array_map('trim', $custom1["ipv6"]);

        # and encoude
        $custom1 = json_encode($custom1, JSON_PRETTY_PRINT);
    };

    # fill custom2
    $custom2 = array();
    if (true) {

        # assign enable
        $custom2["enable"] = false;
        if ($conf['customdns2_enable'] == "on") {
            $custom2["enable"] = true;
        }

        # assign forward fqdn only
        $custom2["forward-fqdn-only"] = false;
        if ($conf['customdns2_forward_fqdn_only'] == "on") {
            $custom2["forward-fqdn-only"] = true;
        }

        # assign ipv4
        $custom2["ipv4"] = explode(",", $conf['customdns2_v4']);
        $custom2["ipv4"] = array_map('trim', $custom2["ipv4"]);

        # assign ipv6
        $custom2["ipv6"] = explode(",", $conf['customdns2_v6']);
        $custom2["ipv6"] = array_map('trim', $custom2["ipv6"]);

        # and encoude
        $custom2 = json_encode($custom2, JSON_PRETTY_PRINT);
    };

    # generate settings
    $settings = array();
    if (true) {

        # assign lookup
        $settings["lookup_etc_resolv_conf"] = false;
        if ($conf['lookup_etc_resolv_conf'] == "on") {
            $settings["lookup_etc_resolv_conf"] = true;
        }

        # and encode
        $settings = json_encode($settings, JSON_PRETTY_PRINT);
    }

    # generate etc hosts
    $hosts = array();
    if (true) {

        # assign lookup
        $hosts["enable"] = false;
        if ($conf['etc_hosts_enable'] == "on") {
            $hosts["enable"] = true;
        }

        # assign path to hosts
        $hosts["hosts"] = explode(",", $conf['etc_hosts_path']);
        $hosts["hosts"] = array_map('trim', $hosts["hosts"]);

        # and encode
        $hosts = json_encode($hosts, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
    }

    # generate data
    $data = "{
        \"settings\" : $settings,
        \"etc-hosts\" : $hosts,
        \"opendns-family-shield\" : $opendns,
        \"google-public-dns\" : $google,
        \"custom1\" : $custom1,
        \"custom2\" : $custom2
    }";

    # and return it
    return $data;
}

//
//
//
class RecategorizedDomains
{
}

function dnssafety_conf_generate_settings($conf) {

    # declare settings
    $settings = array();

    # declare the empty object
    $domains = new RecategorizedDomains();

    # save the empty object into settings
    $settings["recategorized_domains"] = $domains;

    # convert to string
    $data = json_encode($settings, JSON_PRETTY_PRINT);

    # and return it
    return $data;
}

//
//
//
function dnssafety_conf_generate_history($conf){

    # set enable
    $enable = false;
    if (true) {
        if ($conf['enable_history'] == "on") {
            $enable = true;
        }
    }

    // good now, encode addresses as JSON
    $enable = json_encode($enable);
    
    # generate data
    $data = "{
        \"enable\"       : $enable
    }";

    # and return it
    return $data;

}
//
//
//
function dnssafety_conf_generate_cache($conf) {

    # set enable
    $enable = false;
    if (true) {
        if ($conf['enable'] == "on") {
            $enable = true;
        }
    }

    # set positive ttl
    $pttl = $conf['positive-ttl'];
    if (true) {
        if ($pttl == ''){
            $pttl = 3600;
        } else {
            $pttl = intval($pttl);
        }
        if ($pttl <= 0 || $pttl > 3600 * 10) {
            $pttl = 3600;
        }
    }

    # set negative ttl
    $nttl = $conf['negative-ttl'];
    if (true) {
        if ($nttl == ''){
            $nttl = 300;
        } else {
            $nttl = intval($nttl);
        }
        if ($nttl <= 0 || $nttl > 300 * 10) {
            $nttl = 300;
        }
    }

    // good now, encode addresses as JSON
    $enable  = json_encode($enable);
    
    # generate data
    $data = "{
        \"enable\"       : $enable,
        \"positive-ttl\" : $pttl,
        \"negative-ttl\" : $nttl
    }";

    # and return it
    return $data;
}

//
//
//
function dnssafety_conf_generate_arp($conf) {

    # set enable
    $enable = false;
    if (true) {
        if ($conf['enable_arp'] == "on") {
            $enable = true;
        }
    }

    # set no cache
    $nocache = false;
    if (true) {
        if ($conf['arp_nocache'] == "on") {
            $nocache = true;
        }
    }

    # set timeout [0:60]
    $timeout = $conf['arp_timeout'];
    if (true) {
        if ($timeout == ''){
            $timeout = 30;
        } else {
            $timeout = intval($timeout);
        }
        if ($timeout <= 0 || $timeout > 60) {
            $timeout = 30;
        }
    }

    # by default we do arp resolve on all interfaces
    $ifaces = array();

    # fill in the interfaces we will run arp on
    if (true) {

        // for convinience
        $value = $conf['arp_interfaces'];

        // split the interfaces string
        $items = explode(",", $value);

        // loop over each interface            
        foreach ($items as $item) {

            # convert it
            $real = convert_friendly_interface_to_real_interface_name($item);

            # and save
            array_push($ifaces, $real);
        }
    }

    // good now, encode addresses as JSON
    $ifaces  = json_encode($ifaces, JSON_UNESCAPED_SLASHES);
    $enable  = json_encode($enable);
    $nocache = json_encode($nocache);

    # generate data
    $data = "{
        \"enable\"  : $enable,
        \"nocache\" : $nocache,
        \"timeout\" : $timeout,
        \"ifaces\"  : $ifaces
    }";

    # and return it
    return $data;
}

//
// helper function
//
function dnssafety_conf_read_bool($conf, $name) {

    # assume false
    $value = false;    

    # see if it is set to on
    if ($conf[$name] == "on") {
        $value = true;
    }

    # and return it
    return $value;
}

//
// helper function
//      
function dnssafety_conf_split_in_lines($encoded) {

    # declare names
    $items = array();

    # read from config and split into array
    $value = base64_decode($encoded);
    $lines = explode("\n", $value);

    # loop over each line
    foreach ($lines as $line) {

        $item = trim($line);

        if (strlen($item) == 0) {
            continue;
        }

        # save it
        array_push($items, $item);
    }

    # and return nicely
    return $items;
}

//
//
//
function dnssafety_conf_generate_policy_exclusions($conf) {

    $exclusions = array();

    # parse and return
    $exclusions["domain_names"] = dnssafety_conf_split_in_lines($conf['exclusions_domain_names']);
    $exclusions["domain_regex"] = dnssafety_conf_split_in_lines($conf['exclusions_domain_regex']);

    # and return nicely
    return $exclusions;
}

//
//
//
function dnssafety_conf_generate_policy_members($conf) {

    # fill ipv4
    $ipv4 = array();

    if(true) {

        $ipv4["subnets"]   = dnssafety_conf_split_in_lines($conf['member_ipv4subnets']);
        $ipv4["ranges"]    = dnssafety_conf_split_in_lines($conf['member_ipv4ranges']);
        $ipv4["addresses"] = dnssafety_conf_split_in_lines($conf['member_ipv4addrs']);
    }

    # fill ipv6
    $ipv6 = array();

    if (true) {

        $ipv6["subnets"]   = dnssafety_conf_split_in_lines($conf['member_ipv6subnets']);
        $ipv6["ranges"]    = dnssafety_conf_split_in_lines($conf['member_ipv6ranges']);
        $ipv6["addresses"] = dnssafety_conf_split_in_lines($conf['member_ipv6addrs']);
    }

    # fill members
    $members = array();

    if (true) {

        $members["names"] = dnssafety_conf_split_in_lines($conf['member_names']);
        $members["macs"]  = dnssafety_conf_split_in_lines($conf['member_macs']);
        $members["ipv4"]  = $ipv4;
        $members["ipv6"]  = $ipv6;
    }

    # and return nicely
    return $members;
}


//
//
//
function dnssafety_conf_generate_policy_rules($conf) {

    # array of all available categories
    $categories = array(
        "adult_themes_sexuality",
        "advertising",
        "alcohol_tobacco",
        "anime_manga_webcomic",
        "arts_society_culture",
        "automotive",
        "banking_insurance_finance",
        "blogs_personal",            
        "business_services_industry",
        "classifieds_auctions",
        "content_delivery_networks",
        "cracks_hacking_illegal",
        "dating",
        "drugs",
        "ecommerce_shopping",
        "education_science_research",
        "entertainment_humor_hobby",
        "expired_parked",
        "fashion_beauty_cosmetics",            
        "file_storage",            
        "food_dining_restaurants",
        "forums_message_boards",
        "gambling",
        "games",            
        "generic_non_categorized",
        "government",
        "hate_discrimination_violence",
        "health_medicine_fitness",
        "jobs_employment_career",
        "messaging_chat",            
        "movies",
        "music_radio",
        "network_infrastructure",
        "news_media",
        "nudity_pornography",
        "p2p_file_sharing",
        "photo_sharing",            
        "politics",
        "portals",
        "proxy_anonymizer",
        "real_estate_home_interior",
        "religious",            
        "search_engines",            
        "self_harm",
        "social_networking",
        "software_technology_hardware",
        "sports",
        "television",
        "travel",            
        "user_tracking",
        "video_sharing",
        "weapons",
        "webmail"
    );

    # these are arrays with blocked and allowed categories
    $blocked = array();
    $allowed = array();

    # loop over all available categories
    foreach($categories as $category) {

        # read from config
        $name  = "category_{$category}";
        $block = false;    
        if ($conf[$name] == "block") {
            $block = true;
        }

        # and assign to appropriate array
        if ($block) {
            array_push($blocked, $category);
        } else {
            array_push($allowed, $category);
        }
    }

    # cool now construct rules
    $rules = array();

    # fill it in
    $rules["enable"]     = dnssafety_conf_read_bool($conf, 'rules_enable');
    $rules["categories"] = array( 
        "block_noncategorized_domains" => false, # for now not used
        "blocked_categories" => $blocked, 
        "allowed_categories" => $allowed
    );
    $rules["blacklist"] = array( 
        "domain_names"  => dnssafety_conf_split_in_lines($conf['rules_blacklist_domain_names']), 
        "domain_regex"  => dnssafety_conf_split_in_lines($conf['rules_blacklist_domain_regex'])
    );

    # nice return it
    return $rules;
}

//
//
//
function dnssafety_conf_generate_policy($conf) {

    # declare policy
    $policy = array();

    # set name and comment
    if (true) {        
        $policy["name"]    = strtolower(trim($conf['name']));
        $policy["comment"] = strtolower(trim($conf['comment']));
    }

    # set enable
    if (true) {

        # assume not enabled
        $enable = false;    
        if ($conf['enable'] == "on") {
            $enable = true;
        }

        # default policy is always enabled
        if ( $policy["name"] == "default") {
            $enable = true;
        }

        # save it
        $policy["enable"] = $enable;

    }

    $policy["exclusions"] = dnssafety_conf_generate_policy_exclusions($conf);
    $policy["members"]    = dnssafety_conf_generate_policy_members($conf);
    $policy["rules"]      = dnssafety_conf_generate_policy_rules($conf);
    
    # and return the policy
    return $policy;
}

//
//
//
function dnssafety_conf_generate_policies($confs) {

    # declare policies
    $policies = array();

    # loop over the configuration and fill in the policy
    for ($i = 0; $i < sizeof($confs); $i++) {

        # fill in a policy
        $policy = dnssafety_conf_generate_policy($confs[$i]);

        # and append it
        array_push($policies, $policy);
    }

    # convert to string
    $data = json_encode($policies, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);

    # and return it
    return $data;
}

//
//
//
function dnssafety_conf_generate_network($conf) {

    # set port
    $port = $conf['listenport'];
    if ($port == ''){
        $port = '5353';
    }

    # set tcp listen flag
    $tcp = false;
    if ($conf['listentcp'] == "on") {
        $tcp = true;
    }

    # set default listen on all addresses
    $addrs = array("udp://0.0.0.0:$port");
    if ($tcp) {
        array_push($addrs, "tcp://0.0.0.0:$port");
    }

    # fill in the addresses
    if (true) {

        // for convinience
        $value = $conf['listenon'];
        
        // see if we have to listen on all
        if (preg_match("/All/", $value)) {

            // do nothing as we already listening on all by default

        } else {

            // we have to listen on specific interfaces only, so reset the defaults
            $addrs = array();

            // split the interfaces string
            $interfaces = explode(",", $value);

            // loop over each interface            
            foreach ($interfaces as $interface) {

                // if it is an IP address (IPv6 or IPv4 does not matter, just add it to the array)
                if (is_ipaddrv6($interface) || is_ipaddr($interface)) {

                    // specific encodings for IPv6 addresses
                    if (is_ipaddrv6($interface)) {
                        $interface = "[" . $interface . "]";
                    }

                    array_push($addrs, "udp://$interface:$port");
                    if ($tcp) {
                        array_push($addrs, "tcp://$interface:$port");
                    }

                } else {

                    // it is not the IP address, then must be an interface name, look it up
                    $info  = pfSense_get_interface_addresses(convert_friendly_interface_to_real_interface_name($interface));

                    // get the actual values
                    $addr4 = $info['ipaddr'];
                    $addr6 = $info['ipaddr6'];

                    // validate and add them
                    if (is_ipaddr($addr4)) {

                        array_push($addrs, "udp://$addr4:$port");                        
                        if ($tcp) {
                            array_push($addrs, "tcp://$addr4:$port");
                        }
                    }

                    if (is_ipaddrv6($addr6)) {

                        array_push($addrs, "udp://[$addr6]:$port");                        
                        if ($tcp) {
                            array_push($addrs, "tcp://[$addr6]:$port");
                        }
                    }
                }
            }
        }
    }

    // good now, encode addresses as JSON
    $addrs = json_encode($addrs, JSON_UNESCAPED_SLASHES);

    # generate data
    $data = "{
        \"listen\" : {
            \"addresses\" : $addrs
        }
    }";

    # and return it
    return $data;
}

//
// called when "save" button is pressed in UI; also called upon installation
//
function dnssafety_sync() {

    // get the global configuration object
    global $config;

    // mount configuration read-write
    conf_mount_rw();

    # get the dns safety config nodes
    $conf0 = $config['installedpackages']['dnssafety']['config'][0];
    $conf1 = $config['installedpackages']['dnssafetycache']['config'][0];
    $conf2 = $config['installedpackages']['dnssafetyforwarders']['config'][0];
    $conf3 = $config['installedpackages']['dnssafetyfilter']['config'];
    $conf4 = $config['installedpackages']['dnssafetysettings']['config'][0];
    
    # generate various configuration parts
    $network  = dnssafety_conf_generate_network($conf0);
    $logging  = dnssafety_conf_generate_logging($conf0);
    $blocker  = dnssafety_conf_generate_blocker($conf0);
    $arp      = dnssafety_conf_generate_arp($conf0);
    $cache    = dnssafety_conf_generate_cache($conf1);
    $history  = dnssafety_conf_generate_history($conf1);
    $forward  = dnssafety_conf_generate_forwarders($conf2);
    $policies = dnssafety_conf_generate_policies($conf3);
    $settings = dnssafety_conf_generate_settings($conf4);

    # save them into JSON
    $data = "{
        \"logging\"    : $logging,
        \"network\"    : $network,
        \"blocker\"    : $blocker,
        \"arp\"        : $arp,
        \"forwarders\" : $forward,
        \"cache\"      : $cache,
        \"history\"    : $history,
        \"policies\"   : $policies,
        \"settings\"   : $settings
    }";

    # this is the path to configuration file
    $json_conf = "/opt/dnssafety/etc/config.json";
    $json_bak  = "/opt/dnssafety/etc/config.json.bak";

    # backup current conf file
    if (file_exists($json_conf)) {

        # read old data
        $contents = file_get_contents($json_conf);

        # and write to bak file
        @file_put_contents($json_bak, $contents);
    }

    # and write data into current conf
    @file_put_contents($json_conf, $data);

    # shall we also reset owner?
    # chown($json_conf, "daemon");
    # chown($json_bak , "daemon");

    # and finally re-write rc file
    dnssafety_write_rcfile();

    // TODO: sync to backup servers
    // bind_sync_on_changes();

    // mount configuration read-only
    conf_mount_ro();
}

//
// generates the startup script for DNS Safety
//
function dnssafety_write_rcfile() {

    # get the global config
    global $config;

    # get the dns safety config node
    $dnssafety = $config['installedpackages']['dnssafety']['config'][0];

    # see if we are enabled
    $enable = "no";
    if ($dnssafety["enable_dnssafety"] == "on") {
        $enable = "yes";
    }

    # construct the text to write to rc file
    $colon = ": \${dsdnsd_enable:=$enable}";
    $text  = <<<EOD
#!/bin/sh
#
# PROVIDE: dsdnsd
# REQUIRE: networking
# KEYWORD:

. /etc/rc.subr

name="dsdnsd"
rcvar="dsdnsd_enable"
command="/usr/sbin/daemon"
command_args="-P /var/run/dsdnsd.pid -u daemon -o /opt/dnssafety/var/log/dsdnsd.log /opt/dnssafety/bin/dsdnsd"

load_rc_config dsdnsd
$colon

run_rc_command "$1"
EOD;

    # mount configuration read-write
    conf_mount_rw();

    # constants
    $dsdnsd_sh = "/usr/local/etc/rc.d/dsdnsd.sh";
    $dsdnsd_rc = "dsdnsd"

    # save rc text
    @file_put_contents($dsdnsd_sh, $text);

    # make it runnable
    if ($enable == "yes" ) {

        # mark rc file as executable
        @chmod($dsdnsd_sh, 0755);

        # and start the service
        restart_service($dsdnsd_rc);

    } else {

        # otherwise the rc file is not executable
        @chmod($dsdnsd_sh, 0644);

        # and service shall be stopped
        stop_service($dsdnsd_rc);    
    }

    # mount configuration read-only    
    conf_mount_ro();
}

?>

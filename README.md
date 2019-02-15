# pfSense UI Plug-In for DNS Safety Filter

Install FreeBSD 11, add user `builder`. After installation, run the following commands to bootstrap the system.

	pkg install git
	pkg install mc
	pkg install bash
	pkg install sshpass
	pkg install scp

Change shell for `builder` user to `/usr/local/bin/bash` using `chsh` command. Log out and login again.

## Build Package

Login into terminal console and run the following commands to checkout the plugin source code.

	cd /home/builder
	mkdir diladele
	cd diladele
	git clone git@github.com:diladele/dnssafety-pfsense-plugin.git

Now build the plugin by running
	
	cd ~/diladele/dnssafety-pfsense-plugin
	./build.sh

After package is built successfully, use the following command push the prepared package into pfSense (10.0.0.1).

	cd ~/diladele/dnssafety-pfsense-plugin
	sh publish.sh

## Install Package on pfSense

From terminal console run the following commands to reinstall the package.

	pkg delete pfSense-pkg-DnsSafety
	pkg install pfSense-pkg-DnsSafety-0.5.0_1.txz

Logoff from Web UI of pfSense or just hit Ctrl + F5 in the browser.

## Put Package into Online Repo

TODO
# pfSense Plug-In Module for DNS Safety Filter

## Build Package

Assuming you have FreeBSD 11 64-bit installed. Current user is `builder`. Run the following commands to checkout the plugin source code.

	cd /home/builder
	mkdir diladele
	cd diladele
	git clone git@github.com:diladele/dnssafety-pfsense-plugin.git

Now build the plugin.

	cd /home/builder/diladele/dnssafety-pfsense
	cd pfSense-pkg-DnsSafety
	make package

## Put Package into Repo

## Install Package on pfSense




# pfSense UI Plug-In for DNS Safety Filter

## Build Package

Package is built on the latest FreeBSD 11 and then pushed onto the actual pfSense for installation. So create a new FreeBSD machine with `builder` user, login into terminal console and run the following commands to checkout the plugin source code.

	cd /home/builder
	mkdir diladele
	cd diladele
	git clone git@github.com:diladele/dnssafety-pfsense-plugin.git

Now build the plugin by running
	
	cd ~/diladele/dnssafety-pfsense-plugin
	sh build.sh

## Install Package on pfSense

After package is built successfully, use the following command to push it into pfSense (10.0.0.1).

	cd ~/diladele/dnssafety-pfsense-plugin
	sh publish.sh

	cd /root/diladele/dnssafety-pfsense-plugin/pfSense-pkg-DnsSafety/work/pkg
	pkg install pfSense-pkg-DnsSafety-0.5.0_1.txz

To remove the package from the development machine, use the following commands.

	todo?



Upload the `/home/builder/diladele/dnssafety-pfsense-plugin/pfSense-pkg-DnsSafety/work/pkg/pfSense-pkg-DnsSafety-0.5.0_1.txz` to pfSense and run the following command to install it.

	pkg add pfSense-pkg-DnsSafety-0.5.0_1.txz
	reboot


## Put Package into Online Repo

TODO
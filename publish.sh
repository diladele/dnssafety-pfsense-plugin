pushd ./pfSense-pkg-dnssafety/work/pkg
sshpass -p 'pfsense' scp ./pfSense-pkg-dnssafety-0.5.0.txz root@10.0.0.1:/root/
popd

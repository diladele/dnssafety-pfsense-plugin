pushd ./pfSense-pkg-DnsSafety/work/pkg
sshpass -p 'pfsense' scp ./pfSense-pkg-DnsSafety-0.5.0.txz root@10.0.0.1:/root/
popd

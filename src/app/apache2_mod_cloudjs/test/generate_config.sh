current_path=`pwd`

axOS=`uname`
axCPU=`uname -m`

if test ${axCPU} == i386; then axCPU=x86; fi;
if test ${axCPU} == i486; then axCPU=x86; fi;
if test ${axCPU} == i586; then axCPU=x86; fi;
if test ${axCPU} == i686; then axCPU=x86; fi;

axPLATFORM_PATH=${axOS}/${axCPU}


rm modules
if test ${axOS} == Darwin; then
	ln -s /usr/libexec/apache2 modules
	libstdc_filename=/usr/lib/libstdc++.6.dylib
else
	if test ${axCPU} == x86_64; then
		#ln -s /usr/lib64/httpd/modules modules
		cp -Rf /usr/lib64/httpd/modules modules
		libstdc_filename=/usr/lib64/libstdc++.so.6
	else
		#ln -s /usr/lib/httpd/modules modules
		cp -Rf /usr/lib/httpd/modules modules
		libstdc_filename=/usr/lib/libstdc++.so.6
	fi
fi

sed -e "s|<test_root>|${current_path}|g" conf/httpd.conf.in > conf/httpd.conf
sed -e "s|<test_root>|${current_path}|g" start.sh.in > start.sh

sed -e "s|<test_root>|${current_path}|g" \
	-e "s|<axPLATFORM_PATH>|${axPLATFORM_PATH}|g" \
	-e "s|<libstdc_filename>|${libstdc_filename}|g" \
	conf.d/cloudjs.conf.in > conf.d/cloudjs.conf

chmod +x start.sh

./start.sh


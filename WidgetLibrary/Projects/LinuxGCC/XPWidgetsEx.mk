all: XPWidgetsEx32_lib XPWidgetsEx64_lib

clean: clean_XPWidgetsEx

##############################
#######  TARGET: XPWidgetsEx
##############################

XPWidgetsEx32_lib:
	g++ -c -m32 -DLIN=1 -I../../SDK/CHeaders/XPLM/ -I../../SDK/CHeaders/Widgets/ ../../SampleCode/XPListBox.cpp -o ../../SampleCode/XPListBox32.o
	g++ -c -m32 -DLIN=1 -I../../SDK/CHeaders/XPLM/ -I../../SDK/CHeaders/Widgets/ ../../SampleCode/XPPopups.cpp -o ../../SampleCode/XPPopups32.o
	ar -r ../../Binaries/LinuxGCC/XPWidgetsEx32.lib ../../SampleCode/XPListBox32.o ../../SampleCode/XPPopups32.o

XPWidgetsEx64_lib:
	g++ -c -m64 -DLIN=1 -I../../SDK/CHeaders/XPLM/ -I../../SDK/CHeaders/Widgets/ ../../SampleCode/XPListBox.cpp -o ../../SampleCode/XPListBox64.o
	g++ -c -m64 -DLIN=1 -I../../SDK/CHeaders/XPLM/ -I../../SDK/CHeaders/Widgets/ ../../SampleCode/XPPopups.cpp -o ../../SampleCode/XPPopups64.o
	ar -r ../../Binaries/LinuxGCC/XPWidgetsEx64.lib ../../SampleCode/XPListBox64.o ../../SampleCode/XPPopups64.o	
	
clean_XPWidgetsEx:
	rm -f ${OBJS_XPWidgetsEx}

##############################

# end Makefile


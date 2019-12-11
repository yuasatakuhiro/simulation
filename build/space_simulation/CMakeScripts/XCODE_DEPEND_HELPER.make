# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.gnuplot.Debug:
/Users/yuasatakuhiroshi/GitHub/simulation/build/space_simulation/Debug/libgnuplot.a:
	/bin/rm -f /Users/yuasatakuhiroshi/GitHub/simulation/build/space_simulation/Debug/libgnuplot.a


PostBuild.gnuplot.Release:
/Users/yuasatakuhiroshi/GitHub/simulation/build/space_simulation/Release/libgnuplot.a:
	/bin/rm -f /Users/yuasatakuhiroshi/GitHub/simulation/build/space_simulation/Release/libgnuplot.a


PostBuild.gnuplot.MinSizeRel:
/Users/yuasatakuhiroshi/GitHub/simulation/build/space_simulation/MinSizeRel/libgnuplot.a:
	/bin/rm -f /Users/yuasatakuhiroshi/GitHub/simulation/build/space_simulation/MinSizeRel/libgnuplot.a


PostBuild.gnuplot.RelWithDebInfo:
/Users/yuasatakuhiroshi/GitHub/simulation/build/space_simulation/RelWithDebInfo/libgnuplot.a:
	/bin/rm -f /Users/yuasatakuhiroshi/GitHub/simulation/build/space_simulation/RelWithDebInfo/libgnuplot.a




# For each target create a dummy ruleso the target does not have to exist

#!/bin/sh
make -C /Users/yuasatakuhiroshi/GitHub/simulation/build -f /Users/yuasatakuhiroshi/GitHub/simulation/build/CMakeScripts/ALL_BUILD_cmakeRulesBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all

#    Build Configuration Adjust, a source configuration and Makefile
#    generation tool. Copyright © 2013 Stover Enterprises, LLC
#    (an Alabama Limited Liability Corporation), All rights reserved.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

prepare_environment() {
 rm -rf testing_environment
 mkdir testing_environment
 cp configure testing_environment/
 mkdir testing_environment/buildconfiguration
 cp native/buildconfigurationadjust-single-file-distribution.c \
    ./testing_environment/buildconfiguration/buildconfigurationadjust.c
}

graphviz_sanity_check() {
 if [ "${GVTESTS:-YES}" == "NO" ]
 then
  return 0
 fi

 mkdir -p ../gvplottests
 echo -n "test: gvsane$1: " >> ../test.sh-results 
 ./bca --generate-graphviz &> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test gvsane$1:" >&2
  cat out >&2
 elif [ ! -f "./bcaproject.dot" ]
 then
  echo "failed: no bcaproject.dot file created" >> ../test.sh-results
 else 
  dot -Tpng -o bcaproject.png bcaproject.dot
  if [ $? != 0 ]
  then 
   echo "failed: dot returned non-zero" >> ../test.sh-results
  elif [ ! -f "./bcaproject.png" ]
  then
   echo "failed: no bcaproject.png file created" >> ../test.sh-results
  else
   echo "passed" >> ../test.sh-results
   mv ./bcaproject.png ../gvplottests/$1.png
  fi
 fi
}

output_check() {
 grep "BCA: WARNING" out > /dev/null
 if [ $? == 0 ]
 then
  ERROR="warning(s) from BCA"
  return
 fi

 grep "make: Circular" out > /dev/null
 if [ $? == 0 ]
 then
  ERROR="circular dependency in Makefile"
  return
 fi
}

created_files_check() {
 COMPONENTS=`./bca --listprojectcomponents`
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ ! -f $FILE ]
   then
    ERROR="failed: component $COMPONENT file $FILE not created"
    return
   fi
  done
 done
}

makeclean_check() {
 echo -n "test: makeclean$1: " >> ../test.sh-results
 $MAKE -f Makefile.bca clean &> out

 COMPONENTS=`./bca --listprojectcomponents`
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ -f $FILE ]
   then
    echo "failed: $FILE not removed by clean" >> ../test.sh-results
    echo "test.sh: failed test makeclean$1:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 echo "passed" >> ../test.sh-results
}


configurewrapper_buildsinglefiledist() {
 echo -n "test: buildsinglefiledist: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "test" &> out
 ./configure &> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test buildsinglefiledist:" >&2
  cat out >&2
 elif [ ! -f "./bca" ]
 then
  echo "failed: bca not created" >> ../test.sh-results
 else
  echo "passed" >> ../test.sh-results
 fi
 cd ..
}

configurewrapper_useexistingbca() {
 echo -n "test: useexistingbca: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "test" &> out
 ln -s ../native/bca-canadate ./bca
 rm ./buildconfiguration/buildconfigurationadjust.c
 ./configure &> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test useexistingbca:" >&2
  cat out >&2
 else
  echo "passed" >> ../test.sh-results
 fi
 cd ..
}

configurewrapper_userbcafrompath() {
 echo -n "test: usebcafrompath: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "test" &> out
 rm ./buildconfiguration/buildconfigurationadjust.c
 mkdir bin
 cp ../native/bca-canadate ./bin/bca
 PATH=./bin:$PATH ./configure &> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test usebcafrompath:" >&2
  cat out >&2
 elif [ ! -f "./bca" ]
 then
  echo "failed: bca link not created" >> ../test.sh-results
 else
  echo "passed" >> ../test.sh-results
 fi
 cd ..
}

configurewrapper_userbcafromenv() {
 echo -n "test: usebcafromenv: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "test" &> out
 rm ./buildconfiguration/buildconfigurationadjust.c
 BCA=../native/bca-canadate ./configure &> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test usebcafromenv:" >&2
  cat out >&2
 elif [ ! -f "./bca" ]
 then
  echo "failed: bca link not created" >> ../test.sh-results
 else
  echo "passed" >> ../test.sh-results
 fi
 cd ..
}

configurewrapper=(buildsinglefiledist useexistingbca userbcafrompath userbcafromenv)

examples_helloworld() {
 echo -n "test: helloworld: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Hello World" &> out
 ../native/bca-canadate --type BINARY --newvalue NAME hello >> out 2>> out
 ../native/bca-canadate --type BINARY --newvalue FILES hello.c >> out 2>> out
 echo -e "#include <stdio.h>\n\nint main(void)\n{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 ln -sf ../native/bca-canadate ./bca
 ./configure >> out 2>> out
 $MAKE -f Makefile.bca >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test helloworld:" >&2
  cat out >&2
 else
  ERROR=""
  created_files_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2   
  else
   ./native/hello$NATIVEBINSUFFIX >> out 2>> out
   if [ $? != 0 ] 
   then
    echo "failed: hello binary return code wrong" >> ../test.sh-results
   else 
    echo "passed" >> ../test.sh-results
    graphviz_sanity_check helloworld
    makeclean_check helloworld 
   fi
  fi
 fi
 cd ..
}

examples_multifilebin() {
 echo -n "test: mutilfilebin: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Multi File Binary" &> out
 ../native/bca-canadate --type BINARY --newvalue NAME hello >> out 2>> out
 ../native/bca-canadate --type BINARY --newvalue FILES "hello.c main.c" >> out 2>> out
 ../native/bca-canadate --type BINARY --newvalue FILE_DEPENDS "hello.h" >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\nint main(void)\n{\n print_hello();\n return 0;\n}\n" > main.c
 ln -sf ../native/bca-canadate ./bca
 ./configure >> out 2>> out
 $MAKE -f Makefile.bca >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test helloworld:" >&2
  cat out >&2
 else
  ERROR=""
  created_files_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2   
  else
   ./native/hello$NATIVEBINSUFFIX >> out 2>> out
   if [ $? != 0 ] 
   then
    echo "failed: hello binary return code wrong" >> ../test.sh-results
   else 
    echo "passed" >> ../test.sh-results
    graphviz_sanity_check multifilebin
    makeclean_check multifilebin 
   fi
  fi
 fi
 cd ..
}

examples_sharedlib() {
 echo -n "test: sharedlib: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Shared Library" &> out
 ../native/bca-canadate --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ../native/bca-canadate --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ../native/bca-canadate --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ../native/bca-canadate --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 ln -sf ../native/bca-canadate ./bca
 ./configure >> out 2>> out
 $MAKE -f Makefile.bca >> out 2>> out
 filename="./native/${NATIVELIBPREFIX}greetings${NATIVELIBSUFFIX}.0.0"
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test sharedlib:" >&2
  cat out >&2
 else 
  ERROR=""
  created_files_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2   
  else
   ERROR=""
   output_check
   if [ "$ERROR" != "" ]
   then
    echo "failed: $ERROR" >> ../test.sh-results
    echo "test.sh: $ERROR:" >&2
    cat out >&2
   else 
    echo "passed" >> ../test.sh-results
    graphviz_sanity_check sharedlib
    makeclean_check sharedlib 
   fi
  fi
 fi
 cd ..
}

examples_extdepends() {
#this function/test is ment to be run from inside examples_sharedlibandbin()
 echo -n "test: extdepends: " >> ../test.sh-results
 cd ..
 rm -rf testing_environment2
 mkdir testing_environment2
 cp configure testing_environment2/
 mkdir testing_environment2/buildconfiguration
 cp native/buildconfigurationadjust-single-file-distribution.c \
    ./testing_environment2/buildconfiguration/buildconfigurationadjust.c
 cd testing_environment2
 ../native/bca-canadate --newproject "Binary with External Dependency" &> out
 ../native/bca-canadate --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ../native/bca-canadate --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ../native/bca-canadate --component MAIN --type BINARY --newvalue EXT_DEPENDS greetings >> out 2>> out
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 ln -sf ../native/bca-canadate ./bca
 PKG_CONFIG_PATH=../testing_environment/native ./configure >> out 2>> out
 $MAKE -f Makefile.bca >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test extdepends:" >&2
  cat out >&2
 else
  ERROR=""
  created_files_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2   
  else
   export LD_LIBRARY_PATH=../testing_environment/native 
   export DYLD_LIBRARY_PATH=../testing_environment/native
   ./native/main$NATIVEBINSUFFIX >> out 2>> out
   unset LD_LIBRARY_PATH
   unset DYLD_LIBRARY_PATH
   if [ $? != 0 ] 
   then
    echo "failed: binary, main, return code wrong" >> ../test.sh-results
   else 
    ERROR=""
    output_check
    if [ "$ERROR" != "" ]
    then
     echo "failed: $ERROR" >> ../test.sh-results
     echo "test.sh: $ERROR:" >&2
     cat out >&2
    else 
     echo "passed" >> ../test.sh-results
     graphviz_sanity_check extdepends
     makeclean_check extdepends
    fi
   fi
  fi
 fi
 cd ..
 cd testing_environment
}

examples_sharedlibandbin() {
 echo -n "test: sharedlibandbin: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Shared Library and Binary" &> out
 ../native/bca-canadate --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ../native/bca-canadate --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ../native/bca-canadate --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ../native/bca-canadate --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ../native/bca-canadate --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ../native/bca-canadate --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ../native/bca-canadate --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 ln -sf ../native/bca-canadate ./bca
 ./configure >> out 2>> out
 $MAKE -f Makefile.bca >> out 2>> out
 filename="./native/${NATIVELIBPREFIX}greetings${NATIVELIBSUFFIX}.0.0"
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test sharedlibandbin:" >&2
  cat out >&2
 else
  ERROR=""
  created_files_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2   
  else
   export LD_LIBRARY_PATH=./native 
   export DYLD_LIBRARY_PATH=./native
   ./native/main$NATIVEBINSUFFIX >> out 2>> out
   unset LD_LIBRARY_PATH
   unset DYLD_LIBRARY_PATH
   if [ $? != 0 ] 
   then
    echo "failed: binary, main, return code wrong" >> ../test.sh-results
   else 
    ERROR=""
    output_check
    if [ "$ERROR" != "" ]
    then
     echo "failed: $ERROR" >> ../test.sh-results
     echo "test.sh: $ERROR:" >&2
     cat out >&2
    else 
     echo "passed" >> ../test.sh-results
     graphviz_sanity_check sharedlibandbin
     makeclean_check sharedlibandbin 

     #basic check on the disable mechanism for multi-component projects
     rm out
     echo -n "test: sharedlibandbindisable: " >> ../test.sh-results
     ./configure --disable-MAIN >> out 2>> out
     $MAKE -f Makefile.bca >> out 2>> out
     if [ $? != 0 ]
     then
      echo "failed" >> ../test.sh-results
      echo "test.sh: failed test sharedlibandbindisable:" >&2
     else
      ERROR=""
      output_check
      if [ "$ERROR" != "" ]
      then
       echo "failed: $ERROR" >> ../test.sh-results
       echo "test.sh: $ERROR:" >&2
       cat out >&2
      else
       echo "passed" >> ../test.sh-results
       examples_extdepends
      fi
     fi
    fi
   fi
  fi
 fi
 cd ..
}

examples_concat() {
 echo -n "test: concat: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Concatenate" &> out
 ../native/bca-canadate --component textfile --type CAT --newvalue NAME textfile >> out 2>> out
 ../native/bca-canadate --component textfile --type CAT --newvalue FILES "one two three" >> out 2>> out
 echo -n "alpha " > one
 echo -n "beta " > two
 echo -n "gama " > three
 ln -sf ../native/bca-canadate ./bca
 ./configure >> out 2>> out
 $MAKE -f Makefile.bca >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test concat:" >&2
  cat out >&2
 else
  ERROR=""
  created_files_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2   
  else
   grep "alpha  beta  gama" ./native/textfile > /dev/null
   if [ $? != 0 ]
   then
    echo "failed: unexpected textfile contents" >> ../test.sh-results
    echo "test.sh: unexpected textfile contents" >&2
    cat ./native/textfile >&2
   else
    ERROR=""
    output_check
    if [ "$ERROR" != "" ]
    then
     echo "failed: $ERROR" >> ../test.sh-results
     echo "test.sh: $ERROR:" >&2
     cat out >&2
    else
     echo "passed" >> ../test.sh-results
     graphviz_sanity_check concat
     makeclean_check concat 
    fi
   fi
  fi
 fi
 cd ..
}

examples_macroexpand() {
 echo -n "test: macroexpand: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Macro Expansion" &> out
 ../native/bca-canadate --component textfile --type MACROEXPAND --newvalue NAME configure.h >> out 2>> out
 ../native/bca-canadate --component textfile --type MACROEXPAND --newvalue FILES "./configure.h.in" >> out 2>> out
 ../native/bca-canadate --component textfile --type MACROEXPAND --newvalue MAJOR 1 >> out 2>> out
 ../native/bca-canadate --component textfile --type MACROEXPAND --newvalue MINOR 1 >> out 2>> out
 echo -e "#ifndef _CONFIGURE_H_\n#define _CONFIGURE_H_\n"\
         "#define VER_MAJOR \"@BCA.PROJECT.MACROEXPAND.textfile.MAJOR@\"\n"\
         "#define VER_MINOR \"@BCA.PROJECT.MACROEXPAND.textfile.MINOR@\"\n"\
         "#endif" > configure.h.in
 ln -sf ../native/bca-canadate ./bca
 ./configure >> out 2>> out
 $MAKE -f Makefile.bca >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test macroexpand:" >&2
  cat out >&2
 else
  ERROR=""
  created_files_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2   
  else
   grep "VER_MINOR" ./native/configure.h > /dev/null
   if [ $? != 0 ]
   then
    echo "failed: unexpected configure.h contents" >> ../test.sh-results
    echo "test.sh: unexpected configure.h contents" >&2
    cat ./native/textfile >&2
   else
    ERROR=""
    output_check
    if [ "$ERROR" != "" ]
    then
     echo "failed: $ERROR" >> ../test.sh-results
     echo "test.sh: $ERROR:" >&2
     cat out >&2
    else
     echo "passed" >> ../test.sh-results
     graphviz_sanity_check macroexpand
     makeclean_check macroexpand 
    fi
   fi
  fi
 fi
 cd ..
}

examples_generateddeps() {
 echo -n "test: generateddeps: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Generated Dependencies" &> out
 ../native/bca-canadate --component configure --type MACROEXPAND --newvalue NAME configure.h >> out 2>> out
 ../native/bca-canadate --component configure --type MACROEXPAND --newvalue FILES "./configure.h.in" >> out 2>> out
 ../native/bca-canadate --component NONE --type NONE --newvalue MAJOR 1 >> out 2>> out
 ../native/bca-canadate --component NONE --type NONE --newvalue MINOR 1 >> out 2>> out
 ../native/bca-canadate --component sourcefile --type CAT --newvalue NAME hello.c >> out 2>> out
 ../native/bca-canadate --component sourcefile --type CAT --newvalue FILES "one two three" >> out 2>> out
 ../native/bca-canadate  --type BINARY --newvalue NAME hello >> out 2>> out
 ../native/bca-canadate  --type BINARY --newvalue INPUT "configure sourcefile" >> out 2>> out
 echo -e "#ifndef _CONFIGURE_H_\n#define _CONFIGURE_H_\n"\
         "#define VER_MAJOR \"@BCA.PROJECT.BINARY.MAIN.MAJOR@\"\n"\
         "#define VER_MINOR \"@BCA.PROJECT.BINARY.MAIN.MINOR@\"\n"\
         "#endif" > configure.h.in
 echo -e "#include <stdio.h>\n#include \"configure.h\"\n" > one
 echo -e "\nint main(void)\n{\n printf(\"version %s.%s\\\n\", VER_MAJOR, VER_MINOR);" > two
 echo -e "\n return 0;\n}\n" > three
 ln -sf ../native/bca-canadate ./bca
 ./configure >> out 2>> out
 $MAKE -f Makefile.bca >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed" >> ../test.sh-results
  echo "test.sh: failed test generateddeps:" >&2
  cat out >&2
 else
  ERROR=""
  created_files_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2   
  else
   ./native/hello$NATIVEBINSUFFIX >> out 2>> out
   if [ $? != 0 ] 
   then
    echo "failed: hello binary return code wrong" >> ../test.sh-results
   else
    grep "version 1.1" ./out > /dev/null
    if [ $? != 0 ]
    then
     echo "failed: unexpected binary output" >> ../test.sh-results
     echo "test.sh: unexpected binary output" >&2
     cat out >&2
    else
     ERROR=""
     output_check
     if [ "$ERROR" != "" ]
     then
      echo "failed: $ERROR" >> ../test.sh-results
      echo "test.sh: $ERROR:" >&2
      cat out >&2
     else
      echo "passed" >> ../test.sh-results
      graphviz_sanity_check generateddeps
      makeclean_check generateddeps 
     fi
    fi
   fi
  fi
 fi
 cd ..
}

examples_customcommand() {
 echo -n "test: customcommand: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Custom Command" &> out
 ../native/bca-canadate --component ASPELL_WRAPPER --type MACROEXPAND --newvalue NAME aspell_script.sh &>> out
 ../native/bca-canadate --component ASPELL_WRAPPER --type MACROEXPAND --newvalue FILES "aspell_script.sh.in" &>> out
 ../native/bca-canadate --component documentation --type CUSTOM --newvalue NAME documentation.html &>> out
 ../native/bca-canadate --component documentation --type CUSTOM --newvalue FILES documentation.html &>> out
 ../native/bca-canadate --component documentation --type CUSTOM --newvalue DRIVER ASPELL_WRAPPER &>> out
 echo -e "<html>\n <head>\n  <meata http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"\
         " </head>\n <body>\n  <h1>Introduction</h1>\n  <p>Welcome to some great documentation. Abcdef.</p>\n"\
         " </body>\n</html>\n" > documentation.html
 echo -e "#!/bin/bash\nWORDS=\`aspell --mode=sgml --encoding=utf-8 list < \$1\`\n"\
         "if  	[ \"\$WORDS\" != \"\" ]\n"\
         "then\n	echo \"misspelled words in \$1:\"\n	echo \$WORDS\n	exit 1\n"\
         "else\n	cat \$1 > \$2\n	exit 0\nfi\n" > aspell_script.sh.in
 ln -sf ../native/bca-canadate ./bca
 ./configure &>> out
 if [ $? != 0 ]
 then
  echo "failed: configure failed" >> ../test.sh-results
  echo "test.sh: failed test customcommand:" >&2
  cat out >&2
 else
  $MAKE -f Makefile.bca &>> out
  if [ $? == 0 ]
  then
   echo "failed: make should have errored out" >> ../test.sh-results
   echo "test.sh: failed test customcommand:" >&2
   cat out >&2
  else 
   echo -e "<html>\n <head>\n  <meata http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"\
           " </head>\n <body>\n  <h1>Introduction</h1>\n  <p>Welcome to some great documentation. \n"\
           "   This time there are no spelling errors.</p>\n"\
           " </body>\n</html>\n" > documentation.html
   $MAKE -f Makefile.bca &>> out
   if [ $? != 0 ]
   then
    echo "failed: make failed" >> ../test.sh-results
    echo "test.sh: failed test customcommand:" >&2
    cat out >&2
   else
    ERROR=""
    created_files_check
    if [ "$ERROR" != "" ]
    then
     echo "failed: $ERROR" >> ../test.sh-results
     echo "test.sh: $ERROR:" >&2   
    else
     ERROR=""
     output_check
     if [ "$ERROR" != "" ]
     then
      echo "failed: $ERROR" >> ../test.sh-results
      echo "test.sh: $ERROR:" >&2
      cat out >&2
     else
      echo "passed" >> ../test.sh-results
      graphviz_sanity_check customcommand
      makeclean_check customcommand
     fi
    fi
   fi
  fi
 fi
 cd ..
}

examples_inputtocustom() {
 echo -n "test: inputtocustom: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Input to Custom" &> out
 ../native/bca-canadate --component ASPELL_WRAPPER --type MACROEXPAND --newvalue NAME aspell_script.sh &>> out
 ../native/bca-canadate --component ASPELL_WRAPPER --type MACROEXPAND --newvalue FILES "aspell_script.sh.in" &>> out
 ../native/bca-canadate --component documentation --type CUSTOM --newvalue NAME documentation.html &>> out
 ../native/bca-canadate --component documentation --type CUSTOM --newvalue FILES documentation.html &>> out
 ../native/bca-canadate --component documentation --type CUSTOM --newvalue DRIVER ASPELL_WRAPPER &>> out
 ../native/bca-canadate --component documentation2_out --type MACROEXPAND --newvalue NAME documentation2.out &>> out
 ../native/bca-canadate --component documentation2_out --type MACROEXPAND --newvalue FILES documentation2.html.in &>> out
 ../native/bca-canadate --component documentation2 --type CUSTOM --newvalue NAME documentation2.html &>> out
 ../native/bca-canadate --component documentation2 --type CUSTOM --newvalue INPUT documentation2_out &>> out
 ../native/bca-canadate --component documentation2 --type CUSTOM --newvalue DRIVER ASPELL_WRAPPER &>> out
 echo -e "<html>\n <head>\n  <meata http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"\
         " </head>\n <body>\n  <h1>Introduction</h1>\n  <p>Welcome to some great documentation.\n"\
         "   If you think this good, check out page <a href=\"./documentation2.html\">two</a>.</p>\n"\
         " </body>\n</html>\n" > documentation.html
  echo -e "<html>\n <head>\n  <meata http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"\
          " </head>\n <body>\n  <h1>Introduction</h1>\n  <p>Welcome to page 2. \n"\
          "   If this file were to be installed, it would be named @BCA.INSTALLEDNAME.documentation2[0]@.</p>\n"\
          " </body>\n</html>\n" > documentation2.html.in
 echo -e "#!/bin/bash\nWORDS=\`aspell --mode=sgml --encoding=utf-8 list < \$1\`\n"\
         "if  	[ \"\$WORDS\" != \"\" ]\n"\
         "then\n	echo \"misspelled words in \$1:\"\n	echo \$WORDS\n	exit 1\n"\
         "else\n	cat \$1 > \$2\n	exit 0\nfi\n" > aspell_script.sh.in
 ln -sf ../native/bca-canadate ./bca
 ./configure &>> out
 $MAKE -f Makefile.bca &>> out
 if [ $? != 0 ]
 then
  echo "failed: make failed" >> ../test.sh-results
  echo "test.sh: failed test inputtocustom:" >&2
  cat out >&2
 else
  ERROR=""
  output_check
  if [ "$ERROR" != "" ]
  then
   echo "failed: $ERROR" >> ../test.sh-results
   echo "test.sh: $ERROR:" >&2
   cat out >&2
  else
   ERROR=""
   created_files_check
   if [ "$ERROR" != "" ]
   then
    echo "failed: $ERROR" >> ../test.sh-results
    echo "test.sh: $ERROR:" >&2   
   else
    echo "passed" >> ../test.sh-results
    graphviz_sanity_check inputtocustom
    makeclean_check inputtocustom 
   fi
  fi
 fi
 cd ..
}

examples_effectivepaths() {
 echo -n "test: effectivepaths: " >> ./test.sh-results
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "Hello World" &> out
 ../native/bca-canadate --type BINARY --newvalue NAME hello >> out 2>> out
 ../native/bca-canadate --type BINARY --newvalue FILES hello.c >> out 2>> out
 ln -sf ../native/bca-canadate ./bca
 ./configure --prefix=/customdir >> out 2>> out
 rm out
 ../native/bca-canadate --host NATIVE --component MAIN --componenteffectivenames >> out 2>> out
 grep "./native/" out > /dev/null
 if [ $? != 0 ]
 then
  echo "failed: expected LOCAL mode for effective path" >> ../test.sh-results
  echo "test.sh: failed test effectivepaths:" >&2
  cat out >&2
 else
  ../native/bca-canadate --build --host ALL --component ALL --setvalue EFFECTIVE_PATHS INSTALL >> out 2>> out
  if [ $? != 0 ]
  then
   echo "failed" >> ../test.sh-results
   echo "test.sh: failed test effectivepaths:" >&2
   cat out >&2
  else
   rm out
   ../native/bca-canadate --host NATIVE --component MAIN --componenteffectivenames >> out 2>> out
   grep "/customdir/" out > /dev/null
   if [ $? != 0 ]
   then
    echo "failed: expected INSTALL mode for effective path" >> ../test.sh-results
    echo "test.sh: failed test effectivepaths:" >&2
    cat out >&2
   else
    rm out
    ../native/bca-canadate --build --host ALL --component ALL --setvalue EFFECTIVE_PATHS LOCAL >> out 2>> out
    ../native/bca-canadate --host NATIVE --component MAIN --componenteffectivenames >> out 2>> out
    grep "./native/" out > /dev/null
    if [ $? != 0 ]
    then
     echo "failed: expected LOCAL mode again for effective path" >> ../test.sh-results
     echo "test.sh: failed test effectivepaths:" >&2
     cat out >&2
    else
     echo "passed" >> ../test.sh-results
    fi
   fi
  fi
 fi
 cd ..
}

usage() {
 echo -e "usage:\n test.sh\n test.sh suite\n test.sh suite testname\n test.sh --list" >&2
}

examples=(helloworld multifilebin sharedlib sharedlibandbin concat macroexpand generateddeps \
          customcommand inputtocustom effectivepaths)


#script starts here------------------------------------------------
suites=(configurewrapper autoconfsupport configuration examples)

MAKE=make
NATIVEBINSUFFIX=""

if [ $1 == "--help" ]
then
 usage
 exit 0
fi

if [ $1 == "--list" ]
then
 for suite in "${suites[@]}"
 do
  echo "suite: ${suite}"
  array="${suite}[@]"
  for test in "${!array}"
  do
   echo "test: ${test}"
  done
 done
 exit 0
fi

rm -f test.sh-results
rm -rf gvplottests

if [ $# -gt 2 ]
then
 usage
 exit 1
fi

if [ $# == 2 ] 
then
 echo "suite: ${1}" >> ./test.sh-results
 ${1}_${2}
 exit 0
fi

if [ $# == 1 ]
then
 echo "suite: ${1}" >> ./test.sh-results
 array="${1}[@]"
 for test in "${!array}"
 do
  ${1}_${test}
 done
 exit 0
fi

./native/bca-canadate --selftest >> ./test.sh-results
grep failed ./test.sh-results > /dev/null
if [ $? == 0 ]
then
 echo "failures in self test, not proceeding"
 exit 1
fi

for suite in "${suites[@]}"
do
 echo "suite: ${suite}" >> ./test.sh-results
 array="${suite}[@]"
 for test in "${!array}"
 do
  ${suite}_${test}
 done
done
exit 0



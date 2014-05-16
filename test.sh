#    Build Configuration Adjust, a source configuration and Makefile
#    generation tool. Copyright © 2013,2014 Stover Enterprises, LLC
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
 ERROR=""
 rm -rf testing_environment
 mkdir testing_environment
 cp configure testing_environment/
 mkdir testing_environment/buildconfiguration
 cp native/buildconfigurationadjust-single-file-distribution.c \
    ./testing_environment/buildconfiguration/buildconfigurationadjust.c
}

try_configure() {
 ERROR=""
 CFLAGS=$CFLAGS BCA=$BCA PKG_CONFIG_PATH=$PKG_CONFIG_PATH ./configure "$@" >> out 2>> out
 if [ $? != 0 ]
 then
  ERROR="failed: configure"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
 fi
}

try_make() {
 ERROR=""
 $MAKE -f Makefile.bca >> out 2>> out
 if [ $? != 0 ]
 then
  ERROR="failed: make"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
 fi
}

graphviz_sanity_check() {
 ERROR=""
 if [ "${GVTESTS:-YES}" == "NO" ]
 then
  return
 fi

 mkdir -p ../gvplottests
 echo -n "test: ${test}gvsane: " >> ../test.sh-results

 ./bca --generate-graphviz &> out
 if [ $? != 0 ]
 then
  ERROR="failed graphviz sanity check"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}gvsane:" >&2
  cat out >&2
  return
 fi

 if [ ! -f "./bcaproject.dot" ]
 then
  ERROR="failed: no bcaproject.dot file created"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}gvsane:" >&2
  cat out >&2
  return
 fi

 dot -Tpng -o bcaproject.png bcaproject.dot
 if [ $? != 0 ]
 then
  ERROR="failed: dot returned non-zero"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}gvsane:" >&2
  cat out >&2
  return
 fi

 if [ ! -f "./bcaproject.png" ]
 then
  ERROR="failed: no bcaproject.png file created"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}gvsane:" >&2
  cat out >&2
  return
 fi

 echo "passed" >> ../test.sh-results
 mv ./bcaproject.png ../gvplottests/${suite}_${test}.png
}

output_check() {
 ERROR=""
 grep "BCA: WARNING" out > /dev/null
 if [ $? == 0 ]
 then
  ERROR="failed: warning(s) from BCA"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
  return
 fi

 grep "make: Circular" out > /dev/null
 if [ $? == 0 ]
 then
  ERROR="failed: circular dependency in Makefile"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
  return
 fi
}

created_files_check() {
 ERROR=""
 COMPONENTS=`./bca --listprojectcomponents`
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ ! -f $FILE ]
   then
    ERROR="failed: component $COMPONENT file $FILE not created"
    echo $ERROR >> ../test.sh-results
    echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
    cat out >&2
    return
   fi
  done
 done
}

makeclean_check() {
 ERROR=""
 echo -n "test: ${test}makeclean: " >> ../test.sh-results
 $MAKE -f Makefile.bca clean &> out

 COMPONENTS=`./bca --listprojectcomponents`
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ -f $FILE ]
   then
    ERROR="failed: $FILE not removed by clean"
    echo $ERROR >> ../test.sh-results
    echo "test.sh: $ERROR on test ${suite}.${test}makeclean:" >&2
    cat out >&2
    return
   fi
  done
 done
 echo "passed" >> ../test.sh-results
}


configurewrapper_buildsinglefiledist() {
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "test" &> out

 try_configure
 if [ "$ERROR" != "" ]
 then
  return
 fi

 if [ ! -f "./bca" ]
 then
  echo "failed: bca not created" >> ../test.sh-results
 else
  echo "passed" >> ../test.sh-results
 fi
 cd ..
}

configurewrapper_useexistingbca() {
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "test" &> out
 ln -s ../native/bca-canadate ./bca
 rm ./buildconfiguration/buildconfigurationadjust.c

 try_configure
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results

 cd ..
}

configurewrapper_userbcafrompath() {
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "test" &> out
 rm ./buildconfiguration/buildconfigurationadjust.c
 mkdir bin
 cp ../native/bca-canadate ./bin/bca
 PATH=./bin:$PATH

 try_configure
 if [ "$ERROR" != "" ]
 then
  return
 fi

 if [ ! -f "./bca" ]
 then
  echo "failed: bca link not created" >> ../test.sh-results
 else
  echo "passed" >> ../test.sh-results
 fi
 cd ..
}

configurewrapper_userbcafromenv() {
 prepare_environment
 cd testing_environment
 ../native/bca-canadate --newproject "test" &> out
 rm ./buildconfiguration/buildconfigurationadjust.c
 BCA=../native/bca-canadate

 try_configure
 if [ "$ERROR" != "" ]
 then
  return
 fi

 if [ ! -f "./bca" ]
 then
  echo "failed: bca link not created" >> ../test.sh-results
 else
  echo "passed" >> ../test.sh-results
 fi
 cd ..
}

configurewrapper=(buildsinglefiledist useexistingbca userbcafrompath userbcafromenv)

examples_helloworld() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Hello World" &> out
 ./bca --type BINARY --newvalue NAME hello >> out 2>> out
 ./bca --type BINARY --newvalue FILES hello.c >> out 2>> out
 echo -e "#include <stdio.h>\n\nint main(void)\n{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c


 try_configure
 if [ "$ERROR" != "" ]
 then
  return
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 ./native/hello$NATIVEBINSUFFIX >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed: hello binary return code wrong" >> ../test.sh-results
 else
  echo "passed" >> ../test.sh-results
  graphviz_sanity_check helloworld
  makeclean_check helloworld
 fi

 cd ..
}

examples_multifilebin() {
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

 try_configure
 if [ "$ERROR" != "" ]
 then
  return
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 ./native/hello$NATIVEBINSUFFIX >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed: hello binary return code wrong" >> ../test.sh-results
 else
  echo "passed" >> ../test.sh-results
  graphviz_sanity_check multifilebin
  makeclean_check multifilebin
 fi

 cd ..
}

examples_sharedlib() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Shared Library" &> out
 ./bca --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h


 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 graphviz_sanity_check sharedlib
 makeclean_check sharedlib

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

 PKG_CONFIG_PATH=../testing_environment/native
 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 export LD_LIBRARY_PATH=../testing_environment/native
 export DYLD_LIBRARY_PATH=../testing_environment/native
 ./native/main$NATIVEBINSUFFIX >> out 2>> out
 unset LD_LIBRARY_PATH
 unset DYLD_LIBRARY_PATH
 unset PKG_CONFIG_PATH
 if [ $? != 0 ]
 then
  echo "failed: binary, main, return code wrong" >> ../test.sh-results
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 graphviz_sanity_check extdepends
 makeclean_check extdepends

 cd ..
 cd testing_environment
}

examples_sharedlibandbin() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Shared Library and Binary" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c


 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 export LD_LIBRARY_PATH=./native
 export DYLD_LIBRARY_PATH=./native
 ./native/main$NATIVEBINSUFFIX >> out 2>> out
 unset LD_LIBRARY_PATH
 unset DYLD_LIBRARY_PATH
 if [ $? != 0 ]
 then
  echo "failed: binary, main, return code wrong" >> ../test.sh-results
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 graphviz_sanity_check sharedlibandbin
 makeclean_check sharedlibandbin
 cd ..
}

examples_concat() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Concatenate" &> out
 ./bca --component textfile --type CAT --newvalue NAME textfile >> out 2>> out
 ./bca --component textfile --type CAT --newvalue FILES "one two three" >> out 2>> out
 echo -n "alpha " > one
 echo -n "beta " > two
 echo -n "gama " > three


 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 grep "alpha  beta  gama" ./native/textfile > /dev/null
 if [ $? != 0 ]
 then
  echo "failed: unexpected textfile contents" >> ../test.sh-results
  echo "test.sh: unexpected textfile contents" >&2
  cat ./native/textfile >&2
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 graphviz_sanity_check concat
 makeclean_check concat

 cd ..
}

examples_macroexpand() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Macro Expansion" &> out
 ./bca --component textfile --type MACROEXPAND --newvalue NAME configure.h >> out 2>> out
 ./bca --component textfile --type MACROEXPAND --newvalue FILES "./configure.h.in" >> out 2>> out
 ./bca --component textfile --type MACROEXPAND --newvalue MAJOR 1 >> out 2>> out
 ./bca --component textfile --type MACROEXPAND --newvalue MINOR 1 >> out 2>> out
 echo -e "#ifndef _CONFIGURE_H_\n#define _CONFIGURE_H_\n"\
         "#define VER_MAJOR \"@BCA.PROJECT.MACROEXPAND.textfile.MAJOR@\"\n"\
         "#define VER_MINOR \"@BCA.PROJECT.MACROEXPAND.textfile.MINOR@\"\n"\
         "#endif" > configure.h.in


 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 grep "VER_MINOR" ./native/configure.h > /dev/null
 if [ $? != 0 ]
 then
  echo "failed: unexpected configure.h contents" >> ../test.sh-results
  echo "test.sh: unexpected configure.h contents" >&2
  cat ./native/textfile >&2
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 graphviz_sanity_check macroexpand
 makeclean_check macroexpand

 cd ..
}

examples_generateddeps() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Generated Dependencies" &> out
 ./bca --component configure --type MACROEXPAND --newvalue NAME configure.h >> out 2>> out
 ./bca --component configure --type MACROEXPAND --newvalue FILES "./configure.h.in" >> out 2>> out
 ./bca --component NONE --type NONE --newvalue MAJOR 1 >> out 2>> out
 ./bca --component NONE --type NONE --newvalue MINOR 1 >> out 2>> out
 ./bca --component sourcefile --type CAT --newvalue NAME hello.c >> out 2>> out
 ./bca --component sourcefile --type CAT --newvalue FILES "one two three" >> out 2>> out
 ./bca  --type BINARY --newvalue NAME hello >> out 2>> out
 ./bca  --type BINARY --newvalue INPUT "configure sourcefile" >> out 2>> out
 echo -e "#ifndef _CONFIGURE_H_\n#define _CONFIGURE_H_\n"\
         "#define VER_MAJOR \"@BCA.PROJECT.BINARY.MAIN.MAJOR@\"\n"\
         "#define VER_MINOR \"@BCA.PROJECT.BINARY.MAIN.MINOR@\"\n"\
         "#endif" > configure.h.in
 echo -e "#include <stdio.h>\n#include \"configure.h\"\n" > one
 echo -e "\nint main(void)\n{\n printf(\"version %s.%s\\\n\", VER_MAJOR, VER_MINOR);" > two
 echo -e "\n return 0;\n}\n" > three


 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 ./native/hello$NATIVEBINSUFFIX >> out 2>> out
 if [ $? != 0 ]
 then
  echo "failed: hello binary return code wrong" >> ../test.sh-results
  echo "test.sh: unexpected binary output" >&2
  cat out >&2t
  return
 fi

 grep "version 1.1" ./out > /dev/null
 if [ $? != 0 ]
 then
  echo "failed: unexpected binary output" >> ../test.sh-results
  echo "test.sh: unexpected binary output" >&2
  cat out >&2
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 graphviz_sanity_check generateddeps
 makeclean_check generateddeps

 cd ..
}

examples_customcommand() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Custom Command" &> out
 ./bca --component ASPELL_WRAPPER --type MACROEXPAND --newvalue NAME aspell_script.sh >> out 2>&1
 ./bca --component ASPELL_WRAPPER --type MACROEXPAND --newvalue FILES "aspell_script.sh.in" >> out 2>&1
 ./bca --component documentation --type CUSTOM --newvalue NAME documentation.html >> out 2>&1
 ./bca --component documentation --type CUSTOM --newvalue FILES documentation.html >> out 2>&1
 ./bca --component documentation --type CUSTOM --newvalue DRIVER ASPELL_WRAPPER >> out 2>&1
 echo -e "<html>\n <head>\n  <meata http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"\
         " </head>\n <body>\n  <h1>Introduction</h1>\n  <p>Welcome to some great documentation. Abcdef.</p>\n"\
         " </body>\n</html>\n" > documentation.html
 echo -e "#!/bin/sh\nWORDS=\`aspell --mode=sgml --encoding=utf-8 list < \$1\`\n"\
         "if  [ \"\$WORDS\" != \"\" ]\n"\
         "then\n	echo \"misspelled words in \$1:\"\n	echo \$WORDS\n	exit 1\n"\
         "else\n	cat \$1 > \$2\n	exit 0\nfi\n" > aspell_script.sh.in


 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi 

 $MAKE -f Makefile.bca >> out 2>> out
 if [ $? = 0 ]
 then
  ERROR="failed: make should have errored out"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
  return
 fi

 echo -e "<html>\n <head>\n  <meata http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"\
           " </head>\n <body>\n  <h1>Introduction</h1>\n  <p>Welcome to some great documentation. \n"\
           "   This time there are no spelling errors.</p>\n"\
           " </body>\n</html>\n" > documentation.html

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 graphviz_sanity_check customcommand
 makeclean_check customcommand

 cd ..
}

examples_inputtocustom() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Input to Custom" &> out
 ./bca --component ASPELL_WRAPPER --type MACROEXPAND --newvalue NAME aspell_script.sh >> out 2>&1
 ./bca --component ASPELL_WRAPPER --type MACROEXPAND --newvalue FILES "aspell_script.sh.in" >> out 2>&1
 ./bca --component documentation --type CUSTOM --newvalue NAME documentation.html >> out 2>&1
 ./bca --component documentation --type CUSTOM --newvalue FILES documentation.html >> out 2>&1
 ./bca --component documentation --type CUSTOM --newvalue DRIVER ASPELL_WRAPPER >> out 2>&1
 ./bca --component documentation2_out --type MACROEXPAND --newvalue NAME documentation2.out >> out 2>&1
 ./bca --component documentation2_out --type MACROEXPAND --newvalue FILES documentation2.html.in >> out 2>&1
 ./bca --component documentation2 --type CUSTOM --newvalue NAME documentation2.html >> out 2>&1
 ./bca --component documentation2 --type CUSTOM --newvalue INPUT documentation2_out >> out 2>&1
 ./bca --component documentation2 --type CUSTOM --newvalue DRIVER ASPELL_WRAPPER >> out 2>&1
 echo -e "<html>\n <head>\n  <meata http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"\
         " </head>\n <body>\n  <h1>Introduction</h1>\n  <p>Welcome to some great documentation.\n"\
         "   If you think this good, check out page <a href=\"./documentation2.html\">two</a>.</p>\n"\
         " </body>\n</html>\n" > documentation.html
  echo -e "<html>\n <head>\n  <meata http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"\
          " </head>\n <body>\n  <h1>Introduction</h1>\n  <p>Welcome to page 2. \n"\
          " </body>\n</html>\n" > documentation2.html.in
 echo -e "#!/bin/sh\nWORDS=\`aspell --mode=sgml --encoding=utf-8 list < \$1\`\n"\
         "if  	[ \"\$WORDS\" != \"\" ]\n"\
         "then\n	echo \"misspelled words in \$1:\"\n	echo \$WORDS\n	exit 1\n"\
         "else\n	cat \$1 > \$2\n	exit 0\nfi\n" > aspell_script.sh.in

 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi 

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 created_files_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 graphviz_sanity_check inputtocustom
 makeclean_check inputtocustom 

 cd ..
}

examples_effectivepaths() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Hello World" &> out
 ./bca --type BINARY --newvalue NAME hello >> out 2>> out
 ./bca --type BINARY --newvalue FILES hello.c >> out 2>> out
 try_configure --prefix=/customdir
 if [ "$ERROR" != "" ]
 then
  return 1
 fi 
 rm out
 ./bca --host NATIVE --component MAIN --componenteffectivenames >> out 2>> out
 grep "./native/" out > /dev/null
 if [ $? != 0 ]
 then
  echo "failed: expected LOCAL mode for effective path" >> ../test.sh-results
  echo "test.sh: failed test effectivepaths:" >&2
  cat out >&2
 else
  ./bca --build --host ALL --component ALL --setvalue EFFECTIVE_PATHS INSTALL >> out 2>> out
  if [ $? != 0 ]
  then
   echo "failed" >> ../test.sh-results
   echo "test.sh: failed test effectivepaths:" >&2
   cat out >&2
  else
   rm out
   ./bca --host NATIVE --component MAIN --componenteffectivenames >> out 2>> out
   grep "/customdir/" out > /dev/null
   if [ $? != 0 ]
   then
    echo "failed: expected INSTALL mode for effective path" >> ../test.sh-results
    echo "test.sh: failed test effectivepaths:" >&2
    cat out >&2
   else
    rm out
    ./bca --build --host ALL --component ALL --setvalue EFFECTIVE_PATHS LOCAL >> out 2>> out
    ./bca --host NATIVE --component MAIN --componenteffectivenames >> out 2>> out
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

examples_customwithnativetool() {
 #start from scratch
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca

 #start a new bca project by giving it a name
 ./bca --newproject "Custom with Native Tool" &> out

 #add a component named "make_hello" that is binary executable to be
 #build from the C source file make_hello.c, with the output named make_hello
 ./bca --component make_hello --type BINARY --newvalue NAME make_hello >> out 2>> out
 ./bca --component make_hello --type BINARY --newvalue FILES make_hello.c >> out 2>> out

 #Behold, sh script that outputs a C program, that outputs a C program.
 echo "#include <stdio.h>" > make_hello.c
 echo "int main(void)" >> make_hello.c
 echo "{" >> make_hello.c
 echo ' printf("#include <stdio.h>\\n");' >> make_hello.c
 echo ' printf("int main(void)\\n");' >> make_hello.c
 echo ' printf("{\\n");' >> make_hello.c
 echo ' printf(" printf(\"hello world\\\\n\");\\n");' >> make_hello.c
 echo ' printf(" return 0;\\n");' >> make_hello.c
 echo ' printf("}\\n");' >> make_hello.c
 echo ' return 0;' >> make_hello.c
 echo '}' >> make_hello.c

 #add a component named "make_hello_wrapper" that is a macro expanded text file
 #with the output name of make_hello_wrapper.sh from the source file
 #make_hello_wrapper.sh.in
./bca --component make_hello_wrapper --type MACROEXPAND --newvalue NAME \
 make_hello_wrapper.sh >> out 2>&1
 ./bca --component make_hello_wrapper --type MACROEXPAND --newvalue FILES \
 "make_hello_wrapper.sh.in" >> out 2>&1

 #contents of make_hello_world.sh.in
 #The last argument to a custom script will be output name. In this case it
 #should be argv[2] referenced as $2 in bash.
 #We also see how to know what the build output name of the make_hello
 #utility will be.
 echo '#!/bin/sh' >> make_hello_wrapper.sh.in
 echo '@BCA.BUILDOUTPUTNAME.make_hello[0]@ > $2' >> make_hello_wrapper.sh.in

 #next add a CUSTOM component named generated_hello that will use the created file
 #make_hello_wrapper.sh to do its bidding, [M W,which is to generate a file called generated_hello.c
 ./bca --component generated_hello --type CUSTOM --newvalue NAME generated_hello.c >> out 2>&1
 ./bca --component generated_hello --type CUSTOM --newvalue INPUT make_hello >> out 2>&1
 ./bca --component generated_hello --type CUSTOM --newvalue DRIVER make_hello_wrapper >> out 2>&1

 #now we want to actually build the tool-generated source file as part of our project.
 ./bca --component special_hello --type BINARY --newvalue NAME special_hello >> out 2>> out
 ./bca --component special_hello --type BINARY --newvalue INPUT generated_hello >> out 2>> out

 try_configure 
 if [ "$ERROR" != "" ]
 then
  return
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results

 graphviz_sanity_check concat
 makeclean_check concat 

 cd ..
} 

usage() {
 echo -e "usage:\n test.sh\n test.sh suite\n test.sh suite testname\n test.sh --list" >&2
}

examples=(helloworld multifilebin sharedlib sharedlibandbin concat macroexpand generateddeps \
          customcommand inputtocustom effectivepaths customwithnativetool)

enablelogic_disableall() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Component Enable Disable Test" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component alt --type BINARY --newvalue NAME altm >> out 2>> out
 ./bca --component alt --type BINARY --newvalue FILES altmain.c >> out 2>> out
 ./bca --component alt --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > altmain.c

 try_configure --disableall
 if [ "$ERROR" != "" ]
 then
  return 1
 fi 

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 COMPONENTS=`./bca --listprojectcomponents`
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ -f $FILE ]
   then
    echo "failed: $FILE should not have been build; component $COMPONENT should have been disabled" >> ../test.sh-results
    echo "test.sh: failed test enablelogic.disableall:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 echo "passed" >> ../test.sh-results
 #this graph is broken
 graphviz_sanity_check concat
 makeclean_check concat 
 cd ..
}

enablelogic_disableone() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Component Enable Disable Test" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue NAME altm >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue FILES altmain.c >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > altmain.c

 try_configure --disable-ALT
 if [ "$ERROR" != "" ]
 then
  return 1
 fi 

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 COMPONENTS="ALT"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ -f $FILE ]
   then
    echo "failed: $FILE should not have been build; component $COMPONENT should have been disabled" >> ../test.sh-results
    echo "test.sh: failed test enablelogic.disableone:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 COMPONENTS="MAIN greetings"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ ! -f $FILE ]
   then
    echo "failed: $FILE should have been build; component $COMPONENT should have not been disabled" >> ../test.sh-results
    echo "test.sh: failed test enablelogic.disableone:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 echo "passed" >> ../test.sh-results
 graphviz_sanity_check concat
 makeclean_check concat 
 cd ..
}

enablelogic_enableone() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Component Enable Disable Test" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue NAME altm >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue FILES altmain.c >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > altmain.c

 try_configure --disableall --enable-greetings
 if [ "$ERROR" != "" ]
 then
  return 1
 fi 

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 COMPONENTS="ALT MAIN"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ -f $FILE ]
   then
    echo "failed: $FILE should not have been build; component $COMPONENT should have been disabled" >> ../test.sh-results
    echo "test.sh: failed test enablelogic.enableone:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 COMPONENTS="greetings"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ ! -f $FILE ]
   then
    echo "failed: $FILE should have been build; component $COMPONENT should have not been disabled" >> ../test.sh-results
    echo "test.sh: failed test enablelogic.enableone:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 echo "passed" >> ../test.sh-results
 graphviz_sanity_check concat
 makeclean_check concat 
 cd ..
}

enablelogic_defaultdisabled() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Component Enable Disable Test" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue NAME altm >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue FILES altmain.c >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
./bca --component NONE --type NONE --newvalue DISABLES MAIN >> out 2>> out

 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > altmain.c

 try_configure
 if [ "$ERROR" != "" ]
 then
  return 1
 fi 

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 COMPONENTS="MAIN"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ -f $FILE ]
   then
    echo "failed: $FILE should not have been build; component $COMPONENT should have been disabled" >> ../test.sh-results
    echo "test.sh: failed test enablelogic.defaultdisabled:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 COMPONENTS="ALT greetings"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ ! -f $FILE ]
   then
    echo "failed: $FILE should have been build; component $COMPONENT should have not been  disabled" >> ../test.sh-results
    echo "test.sh: failed test enablelogic.defaultdisabled:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 echo "passed" >> ../test.sh-results
 graphviz_sanity_check concat
 makeclean_check concat 
 cd ..
}

enablelogic_enabledefaultdisabled() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Component Enable Disable Test" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue NAME altm >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue FILES altmain.c >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component NONE --type NONE --newvalue DISABLES MAIN >> out 2>> out

 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > altmain.c

 try_configure --enable-MAIN
 if [ "$ERROR" != "" ]
 then
  return 1
 fi 

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 COMPONENTS="MAIN ALT greetings"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ ! -f $FILE ]
   then
    echo "failed: $FILE should have been build; component $COMPONENT should have not been disabled" >> ../test.sh-results
    echo "test.sh: failed test enablelogic.$test:" >&2
    cat out >&2
    return 1
   fi
  done
 done
 echo "passed" >> ../test.sh-results
 graphviz_sanity_check concat
 makeclean_check concat 
 cd ..
}

enablelogic_disableinternaldep() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Component Enable Disable Test" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue NAME altm >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue FILES altmain.c >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out

 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > altmain.c

 ./configure --disable-greetings >> out 2>> out
 if [ $? == 0 ]
 then
  ERROR="failed: configure should have failed on makefile generation"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
 fi

 echo "passed" >> ../test.sh-results

 cd ..
}

enablelogic=(disableall disableone enableone defaultdisabled enabledefaultdisabled disableinternaldep)

#some sanity checks on configure swap logic
swaps_configureerrors() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Component Swap Tests" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue NAME altm >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue FILES altmain.c >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > altmain.c

 #for an example we might want to build component ALT on the to be prepared alternative
 #host configuration, DEBUG. This should fail, since host DEBUG is not yet present.
 ./configure --swap-ALT DEBUG >> out 2>> out
 if [ $? == 0 ]
 then
  ERROR="failed: swapping to non-existing host should have failed"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
  return
 fi

 #now try a regular configure with a different host. For instance a parallel build
 #in a debug configuration
 CFLAGS=-g try_configure --host DEBUG
 if [ "$ERROR" != "" ]
 then
  return
 fi

 rm out
 #now that we do have a host named DEBUG, the above swap proceedure should work
 try_configure --swap-ALT DEBUG
 if [ "$ERROR" != "" ]
 then
  return
 fi

 rm out
 #another thing that should not work is disable and swap at the same time
 ./bca --configure --swap-ALT DEBUG --disable-ALT >> out 2>> out
 if [ $? == 0 ]
 then
  ERROR="failed: disabling and swaping the same component on configure should have failed"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
  return
 fi

 rm out
 #another way that could happen is swapping a component disabled by default
 ./bca --component NONE --type NONE --newvalue DISABLES ALT >> out 2>> out
 ./bca --configure --swap-ALT DEBUG >> out 2>> out
 if [ $? == 0 ]
 then
  ERROR="failed: swaping component disabled by default should have failed"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
  return
 fi

 rm out
 #conversly this should work
 try_configure --swap-ALT DEBUG --enable-ALT
 if [ "$ERROR" != "" ]
 then
  return
 fi

 rm out
 #cleanup the default disable
 ./bca --component NONE --type NONE --removevalue DISABLES >> out 2>> out
 #now reconfigure the DEBUG host, disabling component ALT
 #This is still being swapped so it should fail.
 CFLAGS=-g ./configure --host DEBUG --disable-ALT >> out 2>> out
 if [ $? == 0 ]
 then
  ERROR="failed: disabling component that is swaped to from other host should have failed"
  echo $ERROR >> ../test.sh-results
  echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
  cat out >&2
  return
 fi
 if [ "$ERROR" != "" ]
 then
  return
 fi

 echo "passed" >> ../test.sh-results
 cd ..
}

swaps_simple() {
 prepare_environment
 cd testing_environment
 ln -sf ../native/bca-canadate ./bca
 ./bca --newproject "Component Swap Tests" &> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue NAME greetings >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue FILES hello.c >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue LIB_HEADERS hello.h >> out 2>> out
 ./bca --component greetings --type SHAREDLIBRARY --newvalue INCLUDE_DIRS ./ >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue NAME main >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue FILES main.c >> out 2>> out
 ./bca --component MAIN --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue NAME altm >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue FILES altmain.c >> out 2>> out
 ./bca --component ALT --type BINARY --newvalue INT_DEPENDS greetings >> out 2>> out
 echo -e "#include <stdio.h>\n\nint print_hello(void)\n"\
         "{\n printf(\"hello world\\\n\");\n return 0;\n}\n" > hello.c
 echo -e "#ifndef _hello_h_\n#define _hello_h_\n"\
         "int print_hello(void);\n#endif\n" > hello.h
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > main.c
 echo -e "#include \"hello.h\"\n\nint main(void)\n"\
         "{\n print_hello();\n return 0;\n}\n" > altmain.c

 #for an example we might want to build component ALT on the to be prepared alternative
 #host configuration, DEBUG.

 CFLAGS=-g try_configure --host DEBUG
 if [ "$ERROR" != "" ]
 then
  return
 fi

 try_configure --swap-greetings DEBUG
 if [ "$ERROR" != "" ]
 then
  return
 fi

 try_make
 if [ "$ERROR" != "" ]
 then
  return
 fi

 output_check
 if [ "$ERROR" != "" ]
 then
  return
 fi

 COMPONENTS="MAIN ALT"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --host NATIVE --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ ! -f $FILE ]
   then
    ERROR="failed: $FILE should have been built"
    echo $ERROR >> ../test.sh-results
    echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
    return
   fi
  done
 done

 FILES=`./bca --host NATIVE --component greetings --componentbuildoutputnames`
 for FILE in $FILES
 do
  if [ -f $FILE ]
  then
   ERROR="failed: $FILE should not have been built"
   echo $ERROR >> ../test.sh-results
   echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
   return
  fi
 done

 COMPONENTS="MAIN ALT greetings"
 for COMPONENT in $COMPONENTS
 do
  FILES=`./bca --host DEBUG --component $COMPONENT --componentbuildoutputnames`
  for FILE in $FILES
  do
   if [ ! -f $FILE ]
   then
    ERROR="failed: $FILE should have been built"
    echo $ERROR >> ../test.sh-results
    echo "test.sh: $ERROR on test ${suite}.${test}:" >&2
    return
   fi
  done
 done

 echo "passed" >> ../test.sh-results

 graphviz_sanity_check concat
 makeclean_check concat

 cd ..
}


#now we need swap tests for the cases:
# -native build tool used in cross compile case
# -profiler guided optimization

swaps=(configureerrors simple)


#script starts here------------------------------------------------
suites=(configurewrapper autoconfsupport examples enablelogic swaps)

if [ "$MAKE" = "" ]
then
	MAKE=make
fi
BASEDIR=`pwd`

NATIVEBINSUFFIX=""

if [ "$1" == "--help" ]
then
 usage
 exit 0
fi

if [ "$1" == "--list" ]
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
 suite=${1}
 test=${2}
 echo -n "test: $test: " >> ./test.sh-results
 ${1}_${2}
 exit 0
fi

if [ $# == 1 ]
then
 echo "suite: ${1}" >> ./test.sh-results
 suite=${1}
 array="${1}[@]"
 for test in "${!array}"
 do
  echo -n "test: $test: " >> ./test.sh-results
  ${1}_${test}

  if [ "$ERROR" != "" ]
  then
   cd $BASEDIR
  fi

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
  echo -n "test: $test: " >> ./test.sh-results
  ${suite}_${test}

  if [ "$ERROR" != "" ]
  then
   cd $BASEDIR
  fi

 done
done
exit 0



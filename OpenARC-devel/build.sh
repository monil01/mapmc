#!/bin/sh
#
# build.sh - build script for Cetus.
# [June 6, 2020] To create portable cetus.jar, 
#     - run "make -f configure.mk pbase" first.
#     - run "build.sh bin".
#

# Specify the location of the antlr.jar file for your system.
ANTLR="$PWD/lib/antlr.jar"            #antlr location

if [ -z "$ANTLR" ]; then
  echo "Please define ANTLR in $0"
  exit 1
fi

# Check for java/javac/jar.
for tool in java javac jar; do
  which $tool >/dev/null
  if [ $? -ne 0 ]; then
    echo $tool is not found.
    exit 1
  fi
done

# If jllvm is built, enable LLVM support.
enable_llvm=1
jllvm_jar="$PWD/jllvm/jllvm.jar"
jllvm_libdir="$PWD/jllvm/"
jllvm_lib="$jllvm_libdir/libjllvm"
if [ x`uname -s` = x"Darwin" ]; then
  jllvm_lib="$jllvm_lib.dylib"
else
  jllvm_lib="$jllvm_lib.so"
fi
if [ ! -f "$jllvm_jar" ]; then
  enable_llvm=0
  missing_jllvm_file="$jllvm_jar"
elif [ ! -f "$jllvm_lib" ]; then
  enable_llvm=0
  missing_jllvm_file="$jllvm_lib"
fi

# No change is required for these variables.
CETUSROOT=$PWD
SRC="$CETUSROOT/src/*/*/*.java $CETUSROOT/src/*/*/*/*.java"
PARSER="$CETUSROOT/src/cetus/base/grammars"
# Source files for parser construction.
parser_src="
  CetusCParser.java
  CToken.java
  LineObject.java
  NewCParser.g
  Pragma.java
  Pre.g
  PreprocessorInfoChannel.java
"

# retrieve the value of a config variable from make.header
get_config() {
  make -f make.header -f - <<EOF
all:
	@echo '\$($1)'
EOF
}

case "$1" in
  config)
  echo "Applying configuration..."
  make -f configure.mk base
  if [ $enable_llvm = 0 ]; then
    echo "Disabling LLVM support: $missing_jllvm_file not built"
  else
    echo "Enabling LLVM support..."
    make -f configure.mk llvm
  fi
  ;;
  parser)
  echo "Compiling the parser using ANTLR..."
  #Build the parser
  cd $PARSER
  java -cp $ANTLR:class antlr.Tool $PARSER/Pre.g
  java -cp $ANTLR:class antlr.Tool $PARSER/NewCParser.g
  cd -
  ;;
  compile)
  $0 config
  [ -f $PARSER/NewCParser.java ] || $0 parser
  echo "Compiling the source files..."
  [ -d class ] || mkdir class
  classpath="$ANTLR:class"
  if [ $enable_llvm = 0 ]; then
    # Filter the LLVM backend out of $SRC because LLVM support is not
    # enabled.  Expand the wildcards in $SRC here instead of when $SRC is
    # originally defined so that we pick up .java parser files just
    # generated.
    SRC=`ls -1 $SRC | grep -v '/openacc/codegen/llvmBackend/'`
  else
    classpath="$classpath:$jllvm_jar"
  fi
  javac -g -cp "$classpath" -d class $SRC
  ;;
  jar)
  $0 compile
  echo "Archiving the class files..."
  [ -d lib ] || mkdir lib
  if [ $enable_llvm = 0 ]; then
    # OpenARC's behavior will be confusing if the LLVM backend's .class
    # files are present (from a previous build) but OpenARC's paths are
    # configured with LLVM support disabled.
    rm -rf class/openacc/codegen/llvmBackend
  fi
  jar cf lib/cetus.jar -C class .
  ;;
  javadoc)
  echo "Generating JAVA documents..."
  javadoc -d api cetus $SRC
  ;;
  clean)
  echo "Cleaning up..."
  rm -rf class bin lib/cetus.jar
  ;;
  purge)
  $0 clean
  echo "Purging classes and documents..."
  # removes files under grammars directory except for the source files.
  cd $PARSER; mv $parser_src ..; rm -f *
  cd ..; mv $parser_src grammars/
  cd $CETUSROOT
  rm -rf api manual/html manual/manual.pdf
  ;;  
  bin)
  $0 jar
  echo "Generating a wrapper..."
  [ -d bin ] || mkdir bin
  classpath="$ANTLR:$PWD/lib/cetus.jar"
  library_path_opt=
  if [ $enable_llvm = 1 ]; then
    classpath="$classpath:$jllvm_jar"
    library_path_opt="-Djava.library.path=$jllvm_libdir"
  fi
  cat > bin/openarc << EOF
#!/bin/sh
# This file was generated automatically by build.sh.
java $library_path_opt -cp $classpath -Xmx1g openacc.exec.ACC2GPUDriver \$*
EOF
  chmod 755 bin/openarc
  ;;
  check)
  $0 bin || exit 1
  echo "Compiling JUnit test suite..."
  if [ $enable_llvm = 0 ]; then
    echo 'error: JUnit test suite cannot be run while LLVM support is disabled'
    exit 1
  fi
  if [ X`get_config JUNIT_CLASSPATH` = X ]; then
    echo 'error: JUNIT_CLASSPATH is not defined in make.header'
    exit 1
  fi
  classpath="$ANTLR:class:$jllvm_jar:`get_config JUNIT_CLASSPATH`"
  JUNIT_SRC="$CETUSROOT/test/junit/*/*/*/*.java $CETUSROOT/test/junit/*/*/*.java"
  javac -g -cp "$classpath" -d class $JUNIT_SRC || exit 1
  echo "Running JUnit test suite..."
  java -Djava.library.path=jllvm -cp "$classpath" -ea:cetus... -ea:openacc... -ea:org.jllvm... org.junit.runner.JUnitCore openacc.test.JUnitSuite 2>&1 | tee check.log
  exit ${PIPESTATUS[0]}
  ;;
  *)
  echo "Usage: $0 <target>"
  echo "  <target> is one of the followings:"
  echo "  bin     - compile, jar, and generate a wrapper script."
  echo "  compile - compile the source files."
  echo "  clean   - remove classes, jar, and the wrapper."
  echo "  jar     - archive the class files."
  echo "  javadoc - generate api documents."
  echo "  parser  - rebuild the parser."
  echo "  config  - regenerate the configuration."
  echo "  check   - compile and run JUnit test suite."
  echo "  purge   - remove classes, jar, and the wrapper script,"
  echo "            and also remove parser files generated by Antlr."
  exit 1
  ;;
esac

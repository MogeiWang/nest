import subprocess, sys

def run_doxygen():
    """Run the doxygen make command in the designated"""

    try:
        retcode = subprocess.call("./configure --prefix=./local", shell=True)
        retcode = subprocess.call("make doc", shell=True)
        if retcode < 0:
            sys.stderr.write("doxygen terminated by signal %s" % (-retcode))
    except OSError as e:
        sys.stderr.write("doxygen execution failed: %s" % e)


def generate_doxygen_xml(app):
    """Run the doxygen make commands if we're on the ReadTheDocs server"""

    read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

    if read_the_docs_build:
        sys.stderr.write("Running doxygen build for readthedocs")
        run_doxygen()


def setup(app):
    # Add hook for building doxygen xml when needed
    app.connect("builder-inited", generate_doxygen_xml)



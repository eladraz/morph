import os
import sys
import glob
import unittest
import subprocess

XML_RUNNER_TYPE = 'xml'
TEXT_RUNNER_TYPE = 'text'

try:
	import xmlrunner
except ImportError:
	print "Missing xmlrunner package, install easy_install and get the package."
	print "Get easy_install form: http://pypi.python.org/pypi/setuptools#downloads"
	print "easy_install unittest-xml-reporting"
	sys.exit(-5)

class ExeRunResult(object):
    def __init__(self, retcode, stdout, stderr):
        self.retcode = retcode
        self.stdout = stdout
        self.stderr = stderr

    def __str__(self):
        retcode = self.retcode
        stdout = self.stdout
        stdout_length = len(stdout)
        stderr = self.stderr
        stderr_length = len(stderr)
        return """retcode: %(retcode)s
stdout(%(stdout_length)s):
%(stdout)s
stderr(%(stderr_length)s):
%(stderr)s
""" % locals()

class ExeTestCase(unittest.TestCase):
    def __init__(self, test_compiler, exe):
        self.test_compiler = test_compiler
        self.test_compiler_clrcore_dll = os.path.join(os.path.dirname(exe), 'clrcore.dll') 
        self.exe = exe
        test_name = os.path.basename(exe).replace('.', '_')
        setattr(self, test_name, self.test)
        unittest.TestCase.__init__(self, test_name)

    def __run_exe(self, cmd):
        
        ## Cleanup hack - not neeeded anymore
        ## os.system('del /Q c:\\temp\\testclr\\PrecompiledMethods.dat')

        print 'Cmd: %s' % cmd
        p = subprocess.Popen(cmd, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        stdout, stderr = p.communicate()
        retcode = p.returncode       
        result = ExeRunResult(retcode, stdout, stderr)
        return result
  
    def __run_native_exe(self):
        print 'Running exe (native)'
        result = self.__run_exe(self.exe)
        return result

    def __run_tc_exe(self):
        print 'Running exe (test_compiler)'
        tc_exe_cmd = [self.test_compiler, self.test_compiler_clrcore_dll, self.exe]
        result = self.__run_exe(tc_exe_cmd)
        return result
    
    def __check_mismatch(self, native_exe_result, tc_exe_result):
        print 'Check for mismatch...'
        
        mismatch = False
        if native_exe_result.retcode != tc_exe_result.retcode:
            print 'Found retcode mismatch'
            ##mismatch = True
        
        if native_exe_result.stdout != tc_exe_result.stdout:
            print 'Found stdout mismatch'
            mismatch = True
        
        if native_exe_result.stderr != tc_exe_result.stderr:
            print 'Found stderr mismatch'
            mismatch = True
        
        if mismatch:
            print ''
            print 'Native:'
            print '-' * 10
            print native_exe_result
            print ''
            print "TC:"
            print '-' * 10
            print tc_exe_result
        
            self.assertFalse(mismatch, 'Mismatch found!')
    
    def test(self):
        print ''
        print 40 * '-'
        native_exe_result = self.__run_native_exe()
        tc_exe_result = self.__run_tc_exe()
        self.__check_mismatch(native_exe_result, tc_exe_result)
        print 40 * '='
    
def __generate_suite(test_compiler, exes):
    suite = unittest.TestSuite()
    for exe in exes:
        suite.addTest(ExeTestCase(test_compiler, exe))
    return suite

def run(runner_type, clr_compiler_exe, dotnet_exes, reports_path = ''):
    runners = {
        XML_RUNNER_TYPE  : xmlrunner.XMLTestRunner(output = reports_path, verbose = True),
        TEXT_RUNNER_TYPE : unittest.TextTestRunner(verbosity = 3),
    }
        
    try:
        runner = runners[runner_type]
    except KeyError:
        raise Exception('Invalid runner')
    
    suite = __generate_suite(clr_compiler_exe, dotnet_exes)
    result = runner.run(suite)

    if not result.wasSuccessful():
        raise Exception('Tests failed')
   
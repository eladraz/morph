import os
from glob import glob

MSBUILD_EXE = 'C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\MSBuild.exe'

def __load_sln(path):
    slns = []

    for exedir in os.listdir(path):
            found = glob(os.path.join(path, exedir, '*.sln'))
            if found:
                slns.append(found[0])
    
    return slns

def __filter_sln(filter_tests, slns):
    filtered = []

    if not filter_tests:
        return slns
    
    filter_tests = map(str.lower, filter_tests)
    for sln in slns:
        __path, __junk = os.path.splitext(sln)
        test_name = os.path.basename(__path).lower()
        if test_name in filter_tests:
            filtered.append(sln)
    
    return filtered

def __compile(config, verbose, slns):
    exes = []
    msbuild = MSBUILD_EXE
    if verbose:
        cmd = '%(msbuild)s /p:configuration=%(config)s %(sln)s'
    else:
        cmd = '%(msbuild)s /p:configuration=%(config)s %(sln)s 1>NUL'

    for sln in slns:
        retcode = os.system(cmd % locals())
        if 0 != retcode:
            raise Exception('Unable to compile project', sln)
        
        project_name, __junk__ = os.path.splitext(os.path.basename(sln))
        exe = os.path.join(os.path.dirname(sln), 'bin', config, project_name + '.exe')
        if not os.path.exists(exe):
            raise Exception('Unable to find project exe', exe)
        
        print 'Compiled %(project_name)s' % locals()
        exes.append(exe)

    return exes

def compile(config, verbose, tests_path, filter_tests = []):
    print 'Compiling (config=%(config)s, filter=%(filter_tests)s)...' % locals()
    slns = __filter_sln(filter_tests, __load_sln(tests_path))
    exes = __compile(config, verbose, slns)
    return exes

if '__main__' == __name__:
    compile('debug', false, [])

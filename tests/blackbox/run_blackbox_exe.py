import os
import cli
import compiler
import test_runner

def run_tests(config, report, exes):
    print 'Run tests:', config, report, exes
    pass

if '__main__' == __name__:
    args = cli.parse_args()
    
    tests_path = os.path.join(args.clr_root, 'tests', 'NET')
    
    ## Build
    for config in args.config:
        dotnet_exes = compiler.compile(config, args.verbose, tests_path, args.filter)
    
    ## Run tests
    for config in args.config:
        clr_console_exe = os.path.join(args.clr_root, 'bin', 'exe%(config)s', 'clr_console', 'clr_console.exe') % locals()
        if not os.path.exists(clr_console_exe):
            raise Exception('Unable to find clr console exe', clr_console_exe)

        if args.report_xml:
            test_runner.run('xml', clr_console_exe, dotnet_exes, args.report_xml)
        else:
            test_runner.run('text', clr_console_exe, dotnet_exes)

        print ''
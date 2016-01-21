import argparse

CONFIG_DEBUG   = 'debug'
CONFIG_RELEASE = 'release' 
REPORT_XML = 'xml'
REPORT_TXT = 'text'

def parse_args():
    parser = argparse.ArgumentParser(description = 'Run blackbox tests')
    parser.add_argument('--config', type = str, nargs = '+', choices = [CONFIG_DEBUG, CONFIG_RELEASE], default = [CONFIG_DEBUG, CONFIG_RELEASE])
    parser.add_argument('--clr_root', type = str, required = True)    
    parser.add_argument('--filter', type = str, nargs = '+', default = [])
    parser.add_argument('--report_xml', type = argparse.FileType('w'))
    parser.add_argument('--verbose', action = 'store_true', default = False)
    return parser.parse_args()

if '__main__' == __name__:
    args = parse_args()
    print args
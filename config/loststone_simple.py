import pprint
import os
import sys
import hid
import re
from time import sleep
import argparse
import configparser
from collections import OrderedDict

pp = pprint.PrettyPrinter(indent=4)

HID_REPORT = 0x0
SETTINGS_BASE = 0x00
PROFILE_BASE = 0xff
PROFILE_LEN = 0x14
REPORT_LEN = 0x41 # 64 bits plus the report number

BASE_DIR = os.path.dirname(os.path.realpath(__file__))

parser = argparse.ArgumentParser(
    description='Configure loststone.')

parser.add_argument(
    '--vid', metavar='VID', nargs='?',
    required=False, default="0x1234",
    help='an integer for the accumulator')

parser.add_argument(
    '--pid', metavar='PID', nargs='?',
    required=False, default="0x0006",
    help='an integer for the accumulator')

parser.add_argument(
    '--config_file', metavar='FILE', nargs='?',
    required=False, default=os.path.join(BASE_DIR, "loststone.cfg"),
    help='File containing all settings.')

parser.add_argument(
    '--adns_firmware_file', metavar='FILE', nargs='?',
    required=False, default=os.path.join(BASE_DIR, "adns9500_srom_91.txt"),
    help='ADNS firmware file.')

args = parser.parse_args()

cli_actions = { # values *MUST* match the loststone code
    'SET':        0x0001,
    'GET':        0x0002,
    'LOAD_DATA':  0x0003,
    'GET_DATA':   0x0004,
    'CLEAR':      0x0005,
    'INIT':       0x0006
}

btns = { # values *MUST* match the loststone code
    'LEFT': 0x00,
    'MIDDLE': 0x01,
    'RIGHT': 0x02,
    'FORWARD': 0x03,
    'BACK': 0x04,
    'Z': 0x05,
    'HIGH_RES': 0x06
}

# Since we are accessing the raw hid device I am leaving NXP's default VID/PID
# values.
config = {
    'VID': 0x1234,
    'PID': 0x0006,
    'RELEASE': 0x0000,
    }

# I have no idea why but I like the CPI values In base 10
settings = OrderedDict([
    ('CPI_X', 4320),
    ('CPI_Y', 4320),
    ('CPI_MAX', 5040),
    ('CPI_MIN', 0),
    ('CPI_STEP', 90),
    ('CPI_Z', 0),
    ('CPI_H', 0),
    ('CPI_HR_X', 360),
    ('CPI_HR_Y', 360),
    ('BTN_A', btns['LEFT']),
    ('BTN_B', btns['MIDDLE']),
    ('BTN_C', btns['RIGHT']),
    ('BTN_D', btns['Z']),
    ('BTN_E', btns['HIGH_RES']),
    ('BTN_F', btns['FORWARD']),
    ('BTN_G', btns['BACK']),
    ('LED_ACTION', 0),
    ('VID', 0x192f),
    ('PID', 0x0000),
    ('RELEASE', 0x0000),
    ('PROFILE_DEFAULT', 0),
    ('PROFILE_CURRENT', 0),
    ('ADNS_CRC', 0xbeef),
    ('ADNS_ID', 0x56),
    ('ADNS_FW_LEN', 0x0bfe),
    ('ADNS_FW_OFFSET', 0xF000),
])

profiles = dict()

data = {
    'config': config,
    'settings': settings,
    'profiles': profiles,
    }

def load_adns_firmware( h ):
    print( "Loading ADNS firmware" )

    offset = settings['ADNS_FW_OFFSET']

    reports = list()
    rep_num = 0
    rep_len = 0
    data_len = 59


    with open(args.adns_firmware_file) as f:
        for line in f.readlines():
            if rep_len == 0:
                reports.append([])

            rep_len = rep_len + 1
            reports[rep_num].append(int(line, 16))

            if rep_len == data_len:
                rep_num = rep_num + 1
                rep_len = 0

    for report in reports:
        rep = [0] * REPORT_LEN
        rep[0] = 0x0
        rep[1] = cli_actions['LOAD_DATA']
        rep[2] = (offset >> 8) & 0xff
        rep[3] = offset & 0xff
        rep[4] = len(report)

        offset = offset + rep[4]

        i = 5
        for data in report:
            rep[i] = data
            i = i + 1

        h.write(rep)
        sleep(0.2)
        #ret = h.read(REPORT_LEN)
        #if not ret:
        #    print("Unable to read validation response.")
        #    sys.exit()
#
#        for j in range(rep[4]):
#            if report[j] != ret[j]:
#                print(
#                    "Invalid ADNS firmware value. Should be [%X], is [%X]: " %
#                    (report[j], ret[j] ))
#


def load_profiles( h ):
    print( "Loading profiles" )

    #
    # initialize list 64 8bit values plus one for the report number
    #
    rep = [0] * REPORT_LEN
    p_num = 0

    #
    # Doing one profile at a time to keep thinks simple.
    for profile in profiles.values():

        print("Loading profile %s" % chr(0x41 + p_num))
        p_base = PROFILE_BASE + (p_num * (PROFILE_LEN * 2)) & 0xff
        rep[0] = HID_REPORT
        rep[1] = cli_actions['LOAD_DATA']
        rep[2] = p_base & 0xff
        rep[3] = (p_base >> 8) & 0xff
        rep[4] = len(profile) * 2 # 16 bit

        i = 5
        #
        # Break up any values larger than 8bit's into two and pad ones lower.
        # Nothing we have is larger than 16bit.
        #

        for a, v in profile.items():
            if v < -128 or v > 127:
                rep[i] = v & 0xff
                i = i + 1
                rep[i] = (v >> 8) & 0xff
                i = i + 1
            else:
                rep[i] = v
                i = i + 1
                rep[i] = 0x0
                i = i + 1

        h.write(rep)

        sleep(1)
        #
        # loststone reads the data it just wrote to the eeprom and sends it back.
        # So we are putting it back together and comparing it to what we sent.
        # even though we don't get it still need a bit for the report number.
        #
        print("Validating profile %s" % chr(0x41 + p_num))
        ret = h.read(REPORT_LEN)
        i = 0
        for a, v in profile.items():
            test_val = ret[i+1]
            test_val = (test_val << 8) | ret[i]
            i = i + 2
            if v != test_val:
                print("ERROR: Attribute [%s] was not set correctly. Should be [%X] is [%X]." %
                    (a, v, test_val))

        p_num = p_num + 1

def load_settings( h ):
    print( "Loading settings" )

    #
    # initialize list 64 8bit values plus one for the report number
    #
    rep = [0] * REPORT_LEN
    rep[0] = HID_REPORT
    rep[1] = cli_actions['LOAD_DATA']
    rep[2] = 0x0 # FIXME: change to global var
    rep[3] = 0x0
    rep[2] = SETTINGS_BASE & 0xff
    rep[3] = (SETTINGS_BASE >> 8) & 0xff
    rep[4] = len(settings) * 2 # 16 bit

    #
    # Break up any values larger than 8bit's into two and pad ones lower.
    # Nothing we have is larger than 16bit.
    #
    i = 5
    for a, v in settings.items():
        if v < -128 or v > 127:
            rep[i] = v & 0xff
            i = i + 1
            rep[i] = (v >>8) & 0xff
            i = i + 1
        else:
            rep[i] = v
            i = i + 1
            rep[i] = 0x0
            i = i + 1

    h.write(rep)
    sleep(1)
    #
    # loststone reads the data it just wrote to the eeprom and sends it back.
    # So we are putting it back together and comparing it to what we sent.
    # even though we don't get it still need a bit for the report number.
    #
    print("Validating Settings")
    ret = h.read(REPORT_LEN)
    i = 0
    for a, v in settings.items():
        # Converting back to 16 bit so its easer to understand the values.
        test_val = ret[i+1]
        test_val = (test_val << 8) | ret[i]
        i = i + 2
        if v != test_val:
            print("ERROR: Attribute [%s] was not set correctly. Shoulde be [%X] is [%X]." %
                (a, v, test_val))


# FIXME: this is pretty bad, need to redo this.
def to_int(string):
    reg_hex = re.compile('^\s*(0x[0-9abcdefABCDEF]+)')
    reg_dec = re.compile('^\s*(\d+)')
    reg_wrd = re.compile("^\s*'(\w+)'")

    m = reg_hex.search(string)
    if(m):
        return int(m.group(0), 16)

    m = reg_dec.search(string)
    if(m):
        return int(m.group(0), 10)

    m = reg_wrd.search(string)
    if(m):
        if m.group(1) in btns:
            return btns[m.group(1)]

    # Your config file is F'ed in the A so we are bailing.
    # not usual done in a function but hey.
    sys.exit(3)

def init( h ):
    print( "Clearing *ALL* loststone configurations.")
    print( "The ADNS firmware MUST be reloaded before lost stone will work.")

    rep = [0] * REPORT_LEN
    rep[0] = HID_REPORT
    rep[1] = cli_actions['INIT']

    h.write(rep)
    # FIXME: validate everything was cleared out.
    # The clearMem method of the 25LC library I am using in the lost stone
    # has a void return. So for now we are going on faith.
    # I suppose I could return the entire 64K and make sure each bit is 0xff

def load_config(p):
    retval = True
    for section in [ 'config', 'settings' ]:
        if not p.has_section(section):
            print("The configuration file is missing section \"%s\"." %
                section)
            retval = False
        else:
            for name in p.options(section):
                name = name.upper()
                if name in data[section]:
                    data[section][name] = to_int(p.get(section, name))
                else:
                    print("The attribute \"%s\" in section \"%s\" is not valid." %
                        (name, section))
                    retval = False

    for profile in ['profile_a', 'profile_b', 'profile_c', 'profile_d', 'profile_e']:

        profiles[profile] = OrderedDict()
        i = 0 # TODO find a more pythonic way. islice does not allow assignment.
        for a, v in settings.items():
            profiles[profile][a] = v
            i = i + 1
            if i >= PROFILE_LEN:
                break

        if p.has_section(profile):
            for name in p.options(profile):
                name = name.upper()
                if name in settings:
                    tmp = p.get(profile, name)
                    profiles[profile][name] = to_int(p.get(profile, name))
                else:
                    print("The attribute \"%s\" in section \"%s\" is not valid." %
                        (section, name))
                    retval = False
    if not retval:
        print("Exiting, Nothing has bee programed.")
        sys.exit(2)


if __name__ == '__main__':
    parser = configparser.ConfigParser()
    if os.path.exists(args.config_file):
        try:
            parser.read(args.config_file)
        except ConfigParser.ParsingError as err:
            print('Unable to load settings file: %s' % err)
            sys.exit()
    else:
        print('The settings file %s does not exist' % args.config_file)
        sys.exit(1)

    load_config(parser)

    try:
        h = hid.device(config['VID'], config['PID'])
    except IOError as ex:
        print("Error: %s" % ex)
        sys.exit()

    h.set_nonblocking(False)
    print("Attached")
    print("Manufacturer: %s" % h.get_manufacturer_string())
    print("Product:      %s" % h.get_product_string())
    print("Serial No:    %s" % h.get_serial_number_string())

    load_settings(h)

    load_profiles(h)

    load_adns_firmware(h)


